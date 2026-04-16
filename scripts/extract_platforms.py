#!/usr/bin/env python3

from __future__ import annotations

from collections import deque
from pathlib import Path

from PIL import Image


SOURCE_PATH = Path("platforms.jpg")
OUTPUT_DIR = Path("images/platform/Extracted")
CONTACT_SHEET_PATH = OUTPUT_DIR / "platforms_contact_sheet.png"

GRID_COLUMNS = 5
GRID_ROWS = 4
PADDING = 3
COMPONENT_GAP_ALLOWANCE = 12
SKIPPED_VARIANTS = {12, 13, 15}


def compute_alpha(pixel: tuple[int, int, int]) -> int:
    r, g, b = pixel
    brightness = (r + g + b) / 3.0
    saturation = max(pixel) - min(pixel)

    if brightness >= 205.0 and saturation <= 60.0:
        return 0
    return 255


def bleed_transparent_rgb(image: Image.Image, padding_passes: int = 4) -> Image.Image:
    rgba = image.copy()
    pixels = rgba.load()
    width, height = rgba.size

    for y in range(height):
        for x in range(width):
            r, g, b, a = pixels[x, y]
            if a == 0:
                pixels[x, y] = (0, 0, 0, 0)

    for _ in range(padding_passes):
        source_pixels = rgba.load()
        result = rgba.copy()
        result_pixels = result.load()
        changed = False

        for y in range(height):
            for x in range(width):
                r, g, b, a = source_pixels[x, y]
                if a != 0:
                    continue

                neighbor_colors: list[tuple[int, int, int]] = []
                for ny in range(max(0, y - 1), min(height, y + 2)):
                    for nx in range(max(0, x - 1), min(width, x + 2)):
                        if nx == x and ny == y:
                            continue

                        nr, ng, nb, na = source_pixels[nx, ny]
                        if na > 0 or (na == 0 and (nr != 0 or ng != 0 or nb != 0)):
                            neighbor_colors.append((nr, ng, nb))

                if not neighbor_colors:
                    continue

                average = tuple(
                    round(sum(color[channel] for color in neighbor_colors) / len(neighbor_colors))
                    for channel in range(3)
                )
                result_pixels[x, y] = (*average, 0)
                changed = True

        rgba = result
        if not changed:
            break

    return rgba


def find_components(mask: list[list[bool]]) -> list[tuple[int, int, int, int, int]]:
    height = len(mask)
    width = len(mask[0]) if height else 0
    visited = [[False] * width for _ in range(height)]
    components: list[tuple[int, int, int, int, int]] = []

    for y in range(height):
        for x in range(width):
            if not mask[y][x] or visited[y][x]:
                continue

            queue = deque([(x, y)])
            visited[y][x] = True
            min_x = max_x = x
            min_y = max_y = y
            area = 0

            while queue:
                cx, cy = queue.popleft()
                area += 1
                min_x = min(min_x, cx)
                max_x = max(max_x, cx)
                min_y = min(min_y, cy)
                max_y = max(max_y, cy)

                for nx, ny in ((cx - 1, cy), (cx + 1, cy), (cx, cy - 1), (cx, cy + 1)):
                    if nx < 0 or ny < 0 or nx >= width or ny >= height:
                        continue
                    if visited[ny][nx] or not mask[ny][nx]:
                        continue
                    visited[ny][nx] = True
                    queue.append((nx, ny))

            components.append((min_x, min_y, max_x, max_y, area))

    return components


def intersects_focus_region(
    component: tuple[int, int, int, int, int],
    focus_component: tuple[int, int, int, int, int],
) -> bool:
    min_x, min_y, max_x, max_y, _ = component
    focus_min_x, focus_min_y, focus_max_x, focus_max_y, _ = focus_component

    return not (
        max_x < focus_min_x - COMPONENT_GAP_ALLOWANCE
        or min_x > focus_max_x + COMPONENT_GAP_ALLOWANCE
        or max_y < focus_min_y - COMPONENT_GAP_ALLOWANCE
        or min_y > focus_max_y + COMPONENT_GAP_ALLOWANCE
    )


def trim_cell(image: Image.Image) -> Image.Image:
    width, height = image.size
    rgba = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    rgb_pixels = image.load()
    rgba_pixels = rgba.load()

    for y in range(height):
        for x in range(width):
            color = rgb_pixels[x, y]
            alpha = compute_alpha(color)
            rgba_pixels[x, y] = (color[0], color[1], color[2], alpha)

    component_mask = [[rgba_pixels[x, y][3] > 18 for x in range(width)] for y in range(height)]
    components = find_components(component_mask)
    significant_components = [component for component in components if component[4] >= 120]
    if not significant_components:
        return Image.new("RGBA", (1, 1), (0, 0, 0, 0))

    focus_component = max(significant_components, key=lambda component: component[4])
    kept_components = [
        component
        for component in significant_components
        if intersects_focus_region(component, focus_component)
    ]
    if not kept_components:
        return Image.new("RGBA", (1, 1), (0, 0, 0, 0))

    allowed_pixels = set()
    for min_x, min_y, max_x, max_y, _ in kept_components:
        for y in range(min_y, max_y + 1):
            for x in range(min_x, max_x + 1):
                if rgba_pixels[x, y][3] > 0:
                    allowed_pixels.add((x, y))

    for y in range(height):
        for x in range(width):
            if (x, y) not in allowed_pixels:
                r, g, b, _ = rgba_pixels[x, y]
                rgba_pixels[x, y] = (r, g, b, 0)

    left = max(0, min(component[0] for component in kept_components) - PADDING)
    top = max(0, min(component[1] for component in kept_components) - PADDING)
    right = min(width, max(component[2] for component in kept_components) + PADDING + 1)
    bottom = min(height, max(component[3] for component in kept_components) + PADDING + 1)
    return bleed_transparent_rgb(rgba.crop((left, top, right, bottom)))


def build_contact_sheet(images: list[Image.Image]) -> Image.Image:
    cell_width = max(image.width for image in images)
    cell_height = max(image.height for image in images)
    sheet = Image.new("RGBA", (cell_width * GRID_COLUMNS, cell_height * GRID_ROWS), (24, 24, 28, 255))

    for index, image in enumerate(images):
        column = index % GRID_COLUMNS
        row = index // GRID_COLUMNS
        x = column * cell_width + (cell_width - image.width) // 2
        y = row * cell_height + (cell_height - image.height) // 2
        sheet.alpha_composite(image, (x, y))

    return sheet


def main() -> None:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    source = Image.open(SOURCE_PATH).convert("RGB")
    width, height = source.size
    extracted_images: list[Image.Image] = []

    for row in range(GRID_ROWS):
        for column in range(GRID_COLUMNS):
            left = round(column * width / GRID_COLUMNS)
            top = round(row * height / GRID_ROWS)
            right = round((column + 1) * width / GRID_COLUMNS)
            bottom = round((row + 1) * height / GRID_ROWS)

            index = row * GRID_COLUMNS + column + 1
            output_path = OUTPUT_DIR / f"platform_variant_{index:02d}.png"
            if index in SKIPPED_VARIANTS:
                output_path.unlink(missing_ok=True)
                print(f"skipped {output_path}")
                continue

            cell = source.crop((left, top, right, bottom))
            trimmed = trim_cell(cell)
            trimmed.save(output_path)
            extracted_images.append(trimmed)
            print(f"saved {output_path} {trimmed.size}")

    contact_sheet = build_contact_sheet(extracted_images)
    contact_sheet.save(CONTACT_SHEET_PATH)
    print(f"saved {CONTACT_SHEET_PATH}")


if __name__ == "__main__":
    main()
