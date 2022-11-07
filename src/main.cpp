#include "mongoose/mongoose.h"
#include "webserver.h"
#include "ws_adapter.h"
#include "evse.h"
#include <iostream>

#include <ArduinoOcpp.h>
#include <ArduinoOcpp/Core/Configuration.h>
#include <ArduinoOcpp/Core/FilesystemAdapter.h>
#include <ArduinoOcpp/Platform.h>

#define OCPP_CREDENTIALS_FN (AO_FILENAME_PREFIX "/ocpp-creds.jsn")

struct mg_mgr mgr;
AoMongooseAdapter osock {&mgr};

std::array<Evse, OCPP_NUMCONNECTORS - 1> connectors {{1,2}};

int main() {
    mg_log_set(MG_LL_DEBUG);                            
    mg_mgr_init(&mgr);

    //mgr.dns4.url = "udp://134.95.127.1:53";
    
    mg_http_listen(&mgr, "0.0.0.0:8000", http_serve, NULL);     // Create listening connection

    std::shared_ptr<ArduinoOcpp::FilesystemAdapter> filesystem = 
        ArduinoOcpp::makeDefaultFilesystemAdapter(ArduinoOcpp::FilesystemOpt::Use_Mount_FormatOnFail);
    ArduinoOcpp::configuration_init(filesystem);

    std::shared_ptr<ArduinoOcpp::Configuration<const char *>> ocpp_backend = ArduinoOcpp::declareConfiguration<const char *>(
        "AO_BackendUrl", "", OCPP_CREDENTIALS_FN, true, true, true, true);
    std::shared_ptr<ArduinoOcpp::Configuration<const char *>> ocpp_cbId = ArduinoOcpp::declareConfiguration<const char *>(
        "AO_ChargeBoxId", "", OCPP_CREDENTIALS_FN, true, true, true, true);
    std::shared_ptr<ArduinoOcpp::Configuration<const char *>> ocpp_auth = ArduinoOcpp::declareConfiguration<const char *>(
        "AO_BasicAuthKey", "", OCPP_CREDENTIALS_FN, true, true, true, true);
    std::shared_ptr<ArduinoOcpp::Configuration<const char *>> ocpp_ca = ArduinoOcpp::declareConfiguration<const char *>(
        "AO_CaCert", "", OCPP_CREDENTIALS_FN, true, true, true, true);
    
    if (!ocpp_backend || !ocpp_cbId || ! ocpp_auth || !ocpp_ca) {
        AO_DBG_ERR("init error");
        return 1;
    }

    ArduinoOcpp::configuration_save(); //write empty OCPP credentials file to FS

    osock.setUrl(*ocpp_backend, *ocpp_cbId);
    osock.setAuthToken(*ocpp_auth);
    osock.setCa(*ocpp_ca);

    OCPP_initialize(osock);

    for (unsigned int i = 0; i < connectors.size(); i++) {
        connectors[i].setup();
    }

    std::shared_ptr<ArduinoOcpp::Configuration<int>> wsPing = ArduinoOcpp::declareConfiguration<int>(
        "WebSocketPingInterval", 5, CONFIGURATION_FN, true, true, true, true);
    if (wsPing && *wsPing >= 0) {
        osock.setHeartbeatInterval(*wsPing * 1000);
    }

    bootNotification("Demo Client", "ArduinoOcpp");

    for (;;) {                    // Block forever
        mg_mgr_poll(&mgr, 100);
        OCPP_loop();
        for (unsigned int i = 0; i < connectors.size(); i++) {
            connectors[i].loop();
        }
    }
    mg_mgr_free(&mgr);
    return 0;
}
