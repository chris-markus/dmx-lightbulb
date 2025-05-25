#pragma once

#include "stdint.h"
#include "stdlib.h"
#include <Arduino.h>

enum class WifiState {
    ConnectedPrimary = 0,
    ConnectedBootstrap,
    ScanningPrimary,
    ScanningBootstrap,
    BootstrapAP,
    COUNT
};

typedef void (*OnConnectHandler)(void);
typedef void (*OnDisconnectHandler)(void);

class WifiStateMachine {
public:
    WifiStateMachine(OnConnectHandler on_connect_bootstrap, OnConnectHandler on_connect_primary, OnDisconnectHandler on_disconnect)
        : cb_connect_bootstrap(on_connect_bootstrap)
        , cb_connect_primary(on_connect_primary)
        , cb_disconnect(on_disconnect)
        , is_connected(false)
        , state(WifiState::ScanningBootstrap)
        , last_state(WifiState::ScanningPrimary)
        , last_state_change(0UL)
    {}

    void initialize();
    void tick();

    const String ip_address();

    bool connected() {
        return is_connected;
    }

private:
    OnConnectHandler cb_connect_bootstrap;
    OnConnectHandler cb_connect_primary;
    OnDisconnectHandler cb_disconnect;
    bool is_connected;
    WifiState state;
    WifiState last_state;
    unsigned long last_state_change;
};
