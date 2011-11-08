#include <rsiface/rsiface.h> 
#include <rsiface/rsinit.h>  

#include "notifytxt.h"

#include <iostream>
#ifdef WINDOWS_SYS
#include <winsock2.h>
#endif

void authenticate();

int main(int argc, char** argv) {
    RsInit::InitRsConfig();
    RsInit::InitRetroShare(argc, argv);

    authenticate();
    
    /* Now setup the libretroshare interface objs 
     * You will need to create you own NotifyXXX class
     * if you want to receive notifications of events */

    NotifyTxt *notify = new NotifyTxt();
    RsIface *iface = createRsIface(*notify);
    RsControl *rsServer = createRsControl(*iface, *notify);

    notify->setRsIface(iface);

    /* Start-up libretroshare server threads */

    rsServer->StartupRetroShare();

    rsicontrol = rsServer ;
    //CleanupRsConfig(config);

    /* pass control to the GUI */
    while(1)
    {
#ifndef WINDOWS_SYS
        sleep(1);
#else
        Sleep(1000);
#endif
    }
    return 1;


}

void authenticate() {

    std::string gpgId, gpgName, gpgEmail, sslName, accountId, gpgPasswd, passwd;
    accountId = "c8167134ba79c111efb1023e5ab1c910";
    if (RsInit::getAccountDetails(accountId,
                gpgId, gpgName, gpgEmail, sslName))
    {
        RsInit::SelectGPGAccount(gpgId);
        RsInit::LoadGPGPassword("squareisle");
    }
    RsInit::LoadPassword(accountId, passwd);
    RsInit::LoadCertificates(true);
}

