#!/usr/bin/env python3

from __future__ import annotations

from collections import deque
from pathlib import Path

from PIL import Image


SOURCE_PATH = Path("nameplates.jpg")
OUTPUT_DIR = Path("images/decoration/Nameplates")
CONTACT_SHEET_PATH = OUTPUT_DIR / "nameplates_contact_sheet.png"

GRID_COLUMNS = 5
GRID_ROWS = 4
BACKGROUND_COLOR = (28, 29, 31)
ALPHA_THRESHOLD = 8.0
ALPHA_FULL = 18.0
PADDING = 4


def color_distance(pixel: tuple[int, int, int], background: tuple[int, int, int]) -> float:
    return (
        ((pixel[0] - background[0]) ** 2)
        + ((pixel[1] - background[1]) ** 2)
        + ((pixel[2] - background[2]) ** 2)
    ) ** 0.5


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


def alpha_from_distance(distance: float) -> int:
    if distance <= ALPHA_THRESHOLD:
        return 0
    if distance >= ALPHA_FULL:
        return 255
    return int(round((distance - ALPHA_THRESHOLD) * 255.0 / (ALPHA_FULL - ALPHA_THRESHOLD)))


def erase_cell_numbers(image: Image.Image) -> None:
    width, height = image.size
    pixels = image.load()
    zone_left = max(0, width - 82)
    zone_top = max(0, height - 72)

    candidate_mask = [[False] * width for _ in range(height)]
    for y in range(zone_top, height):
        for x in range(zone_left, width):
            r, g, b = pixels[x, y]
            brightness = max(r, g, b)
            saturation = max(r, g, b) - min(r, g, b)
            if brightness >= 150 and saturation <= 135:
                candidate_mask[y][x] = True

    components = find_components(candidate_mask)
    for min_x, min_y, max_x, max_y, area in components:
        sits_in_number_zone = min_x >= width - 82 and min_y >= height - 45 and area <= 600
        if not sits_in_number_zone:
            continue

        for y in range(max(0, min_y - 6), min(height, max_y + 7)):
            for x in range(max(0, min_x - 6), min(width, max_x + 7)):
                pixels[x, y] = BACKGROUND_COLOR


def erase_bottom_right_numbers(image: Image.Image) -> None:
    width, height = image.size
    pixels = image.load()
    zone_left = max(0, width - 76)
    zone_top = max(0, height - 58)

    candidate_mask = [[False] * width for _ in range(height)]
    for y in range(zone_top, height):
        for x in range(zone_left, width):
            r, g, b, a = pixels[x, y]
            if a == 0:
                continue
            brightness = max(r, g, b)
            saturation = max(r, g, b) - min(r, g, b)
            if brightness >= 120 and saturation <= 95:
                candidate_mask[y][x] = True

    components = find_components(candidate_mask)
    pixels_to_clear: set[tuple[int, int]] = set()
    for min_x, min_y, max_x, max_y, area in components:
        sits_in_number_zone = min_x >= width - 58 and min_y >= height - 58
        if not sits_in_number_zone or area > 350:
            continue

        for y in range(max(0, min_y - 5), min(height, max_y + 6)):
            for x in range(max(0, min_x - 5), min(width, max_x + 6)):
                pixels_to_clear.add((x, y))

    for x, y in pixels_to_clear:
        r, g, b, _ = pixels[x, y]
        pixels[x, y] = (r, g, b, 0)


def clear_index_zone(image: Image.Image, index: int) -> Image.Image:
    width, height = image.size
    pixels = image.load()
    zone_width = 48 if index < 10 else 68
    zone_height = 42
    left = max(0, width - zone_width)
    top = max(0, height - zone_height)

    for y in range(top, height):
        for x in range(left, width):
            r, g, b, _ = pixels[x, y]
            pixels[x, y] = (r, g, b, 0)

    alpha_bounds = image.getbbox()
    if alpha_bounds is None:
        return image
    return image.crop(alpha_bounds)


def trim_cell(image: Image.Image) -> Image.Image:
    width, height = image.size
    rgb = image.convert("RGB")
    erase_cell_numbers(rgb)
    pixels = rgb.load()

    mask = [[False] * width for _ in range(height)]
    for y in range(height):
        for x in range(width):
            if color_distance(pixels[x, y], BACKGROUND_COLOR) > ALPHA_THRESHOLD:
                mask[y][x] = True

    components = find_components(mask)
    kept_components: list[tuple[int, int, int, int, int]] = []
    for component in components:
        min_x, min_y, max_x, max_y, area = component
        in_bottom_right = min_x >= int(width * 0.60) and min_y >= int(height * 0.60)
        if in_bottom_right and area < 700:
            continue
        kept_components.append(component)

    if not kept_components:
        return Image.new("RGBA", (1, 1), (0, 0, 0, 0))

    left = max(0, min(component[0] for component in kept_components) - PADDING)
    top = max(0, min(component[1] for component in kept_components) - PADDING)
    right = min(width, max(component[2] for component in kept_components) + PADDING + 1)
    bottom = min(height, max(component[3] for component in kept_components) + PADDING + 1)

    trimmed_rgb = rgb.crop((left, top, right, bottom))
    trimmed_rgba = Image.new("RGBA", trimmed_rgb.size, (0, 0, 0, 0))
    trimmed_rgb_pixels = trimmed_rgb.load()
    trimmed_rgba_pixels = trimmed_rgba.load()

    for y in range(trimmed_rgb.size[1]):
        for x in range(trimmed_rgb.size[0]):
            color = trimmed_rgb_pixels[x, y]
            alpha = alpha_from_distance(color_distance(color, BACKGROUND_COLOR))
            trimmed_rgba_pixels[x, y] = (color[0], color[1], color[2], alpha)

    erase_bottom_right_numbers(trimmed_rgba)
    return trimmed_rgba


def build_contact_sheet(images: list[Image.Image]) -> Image.Image:
    cell_width = max(image.width for image in images)
    cell_height = max(image.height for image in images)
    sheet = Image.new("RGBA", (cell_width * GRID_COLUMNS, cell_height * GRID_ROWS), (24, 25, 28, 255))

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

            cell = source.crop((left, top, right, bottom))
            trimmed = trim_cell(cell)
            index = row * GRID_COLUMNS + column + 1
            trimmed = clear_index_zone(trimmed, index)
            output_path = OUTPUT_DIR / f"nameplate_{index:02d}.png"
            trimmed.save(output_path)
            extracted_images.append(trimmed)
            print(f"saved {output_path} {trimmed.size}")

    contact_sheet = build_contact_sheet(extracted_images)
    contact_sheet.save(CONTACT_SHEET_PATH)
    print(f"saved {CONTACT_SHEET_PATH}")


if __name__ == "__main__":
    main()
