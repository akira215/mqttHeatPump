#pragma once



// Main class used for testing only
class Main final
{

public:
    Main();
    void run(void);
    void setup(void);

private:
    void wifi_init_sta(void);
    //DriverHomebus*  _hbs    = nullptr;


}; // Main Class