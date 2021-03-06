#include <retroshare/rsiface.h> 
#include <retroshare/rsinit.h>  
#include <event.h>
#include <event2/thread.h>
#include <iostream>
#include <locale>

#include <QThreadPool>

#include "raas.h"
#include "raas_web.h"
#include "notifytxt.h"

RsControl* rsweb::rs_control = NULL;
bool rsweb::rs_control_startup_retroshare_called = false;

int main(int argc, char** argv) {
    RsInit::InitRsConfig();
    int initResult = RsInit::InitRetroShare(argc, argv);

    if (initResult < 0) {
        /* Error occured */
        switch (initResult) {
            case RS_INIT_AUTH_FAILED:
                std::cerr << "RsInit::InitRetroShare AuthGPG::InitAuth failed" << std::endl;
                break;
            default:
                /* Unexpected return code */
                std::cerr << "RsInit::InitRetroShare unexpected return code " << initResult << std::endl;
                break;
        }
        return 1;
    }

    /* Now setup the libretroshare interface objs 
     * You will need to create you own NotifyXXX class
     * if you want to receive notifications of events */

    NotifyTxt *notify = new NotifyTxt();
    RsIface *iface = createRsIface(*notify);
    rsweb::rs_control = createRsControl(*iface, *notify);
    
    // this is a global hidden away in libretroshare somewhere
    // FIXME: i dont actually know if we need to set it this early...
    rsicontrol = rsweb::rs_control ;
    notify->setRsIface(iface);

    // !!! we don't call StartupRetroshare() in main()
    // it's not called until the user selects a profile
    // via the web interface and actually activates it
    // doing anything without calling this first results in a segfault
    //rsweb::rs_control->StartupRetroShare();
    
    rsweb::thread_pool& thread_pool = rsweb::get_thread_pool(); 
    
    
    event_init();
    evthread_use_pthreads();
    event_base* evbase = event_base_new();
    evthread_make_base_notifiable(evbase);
    evhttp* http_base = evhttp_new(evbase);
    evhttp_set_gencb(http_base, rsweb::queue_request, &thread_pool); 
    assert(evhttp_bind_socket(http_base, "0.0.0.0", 10101) == 0);

    event_base_dispatch(evbase);
    thread_pool.waitForDone();
    return 1;
}


