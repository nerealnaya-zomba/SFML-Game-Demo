#include<iostream>


// Представляет собой задачу для уровня.
// Не имеет графической оболочки
class LevelTask{
private:
    enum TaskType{KILL,COLLECT,SURVIVE};

    TaskType taskType;

    //System
        //Bool
        bool isComplete;
        //Float
        float target;

    //Update
    void update();
        //Update certain target
        void updateKill();
        void updateCollect();
        void updateSurvive();
public:
    LevelTask(TaskType tt, float taskTarget);
    ~LevelTask() = default;

    //Interaction


    //Setters
    

    //Getters
    bool isCompleted();

};