#pragma once
#include "CampusTimeModel.h"
#include "CampusOperations.h"

struct FCampusSavedPoint { double X=0,Y=0,Z=0; };
struct FCampusVisualSave
{
    std::int64_t Id=0, Day=-1;
    bool Started=false;
    int People=8;
    FCampusReservation Reservation{};
    double Start=0, Arrival=0, End=0, Travel=0, Animation=0;
    int RouteCount=0;
    FCampusSavedPoint Route[10001]{};
};
struct FCampusSaveState
{
    FCampusTimeModel Clock;
    FCampusWeeklySchedule Schedule;
    FCampusOperations Operations;
    bool Built=false, Selected=false;
    double BuildingX=700, BuildingY=0;
    int Turns=0;
    unsigned char Paths[10000]{};
    double PathReadyMinute=0;
    double CameraX=700, CameraY=0, CameraZoom=9000, CameraYaw=-35, CameraPitch=-55;
    FCampusVisualSave Visual;
};
