/** 
 * Generated by DDSCS                                                    *
 * Example client. Method params should be initialized before execution  *
 */

#include "UnionTestProxy.h"
#include "UnionTestRequestReplyPlugin.h"

int main()
{
    int domainId = 0;
    unsigned int timeoutInMillis = 4000;
    UnionTestProxy *proxy = new UnionTestProxy(domainId, timeoutInMillis,
"UnionTest_Library", "UnionTest_Profile");
    DDS_Duration_t period = {5,0};
    char buffer[30];
    
    Empleado *em1 = EmpleadoPluginSupport_create_data();       
    Empleado *em2 = EmpleadoPluginSupport_create_data();       
    Empleado *em3 = EmpleadoPluginSupport_create_data();    
    Empleado *getEmpleado_ret = EmpleadoPluginSupport_create_data();       
    DDSCSMessages  getEmpleadoRetValue ;

    em1->_d = 1;
    em1->_u.id = 1;
    em2->_d = 2;
    em2->_u.name = DDS_String_dup("PRUEBA");

    /**
     * Dynamic memory passed to the proxy will be freed before return *
     * Pass a copy if you want to keep it                             *
     */
    getEmpleadoRetValue = proxy->getEmpleado(*em1    ,*em2    ,*em3    , *getEmpleado_ret    );
    printf("getEmpleado: em2 = %s, em3 = %s, ret = %s\n",
        (em2->_d == 1 ? itoa(em2->_u.id, buffer, 10) : em2->_u.name),
        (em3->_d == 1 ? itoa(em3->_u.id, buffer, 10) : em3->_u.name),
        (getEmpleado_ret->_d == 1 ? itoa(getEmpleado_ret->_u.id, buffer, 10) : getEmpleado_ret->_u.name));
    
    EmpleadoPluginSupport_destroy_data(em1);    
    EmpleadoPluginSupport_destroy_data(em2);    
    EmpleadoPluginSupport_destroy_data(em3);    
    EmpleadoPluginSupport_destroy_data(getEmpleado_ret);    

   delete(proxy);
   NDDSUtility::sleep(period);
}
