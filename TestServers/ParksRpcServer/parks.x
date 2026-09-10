struct Park {
    unsigned int id;
    string name<64>;
    string state<4>;
    double latitude;
    double longitude;
    unsigned int established_year;
    unsigned int area_acres;
};

struct FindNearestArgs {
    double latitude;
    double longitude;
};

struct FindNearestResult {
    Park park;
    double distance_miles;
};

struct ParkList {
    Park parks<>;
};

union GetByNameResult switch (bool found) {
case TRUE:
    Park park;
case FALSE:
    void;
};

program PARKS_PROG {
    version PARKS_VERS {
        GetByNameResult GETBYNAME(string) = 1;
        FindNearestResult FINDNEAREST(FindNearestArgs) = 2;
        ParkList LISTALL(void) = 3;
    } = 1;
} = 0x20000001;
