#pragma once

#include "LedInterface.h"
#include "WiFiStateMachine.h"

class InstanceManager 
{
public:
    explicit InstanceManager(LedInterface &led, WifiStateMachine &wifi) : led_interface(led), wifi_mgr(wifi) {}

    LedInterface &led_interface;
    WifiStateMachine &wifi_mgr;
};

extern InstanceManager globals;
