#pragma once
// Local coordinates relative to the hall's ground centre; matches the original work map.
struct FCampusGymPart { double X, Y, Z, SizeX, SizeY, SizeZ; int Material; };
inline constexpr FCampusGymPart CampusGymParts[] = {
    {0,0,10,4000,2600,20,0}, {0,1300,410,4100,40,800,0}, {0,-1300,410,4100,40,800,0},
    {2000,0,410,40,2600,800,0}, {-2000,800,410,40,1000,800,0}, {-2000,-800,410,40,1000,800,0},
    {0,0,22,2800,1500,4,1}, {0,-750,25,2810,5,2,2}, {0,750,25,2810,5,2,2},
    {-1400,0,25,5,1500,2,2}, {0,0,25,5,1500,2,2}, {1400,0,25,5,1500,2,2}
};
