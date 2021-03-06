/*
CLIENT/SERVER DEMO EXAMPLE: Simple OPC UA Client accessing fake sensor data.

The OPC UA Client sets up a connection to a OPC UA Server by a TCP connection.
The server counts parts with a sensor and updates the numberOfparts attribute 
of its "Piece counter" node. The client accesses this data and prints it insinde a loop.
*/

//library amalgamated to a single file .h/.c file for simplicity
#include "open62541.h"

#include <stdlib.h>


//NodeID of sensor-node
#define COUNTER_NODE_ID 20305
//polling interval for the client
#define SLEEP_TIME_MILLIS 2000000


int main(void) {

/* First, we set up the connection */

	// create a new client
	UA_Client* client = UA_Client_new();
	//set client config to default
	UA_ClientConfig_setDefault(UA_Client_getConfig(client));
	//statuscode for client connection to spcified endpoint (Rpi3 ip-adress)
	UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://10.53.24.102:4840");
	
	if(retval != UA_STATUSCODE_GOOD) {
		UA_Client_delete(client);
		return (int)retval;
	}


/* Next we will read the value attribute of the node */

	// Variants can hold scalar values and arrays of any type 
	UA_Int32 myInteger = 0;
	UA_Variant value; 
	UA_Variant_init(&value);
	UA_Variant_setScalar(&value, &myInteger, &UA_TYPES[UA_TYPES_INT32]);

    // Browsing nodes in objects folder
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);

    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    // browse objects folder 
    bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  	// return everything 
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; 
    UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
    
    /* iterate through list looking for a specific node, the node representing 
       number of counted parts */
    for(size_t i = 0; i < bResp.resultsSize; ++i) {
        for(size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
      
            if( ref->nodeId.nodeId.identifier.numeric == COUNTER_NODE_ID ){
    		
    			while(retval == UA_STATUSCODE_GOOD){
		        	/*READ FROM NODE OPERATION*/
					//= reads the value of the node ID , puts it into "value" 
					retval = UA_Client_readValueAttribute(client, ref->nodeId.nodeId, &value);
					if(retval == UA_STATUSCODE_GOOD && 
					   UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32])) 
					{
						UA_Int32 raw_val = *(UA_Int32*) value.data;
						printf("\nRecieved from server: [Counted pieces] %i ", raw_val);
					}
					else{
						/*For debugging*/
						if(retval != UA_STATUSCODE_GOOD) printf("\nretval != UA_STATUSCODE_GOOD\n");
						if(!UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32])){
							printf("\nUA_Variant_hasScalarType != OK\n");
						}
				    }
				    usleep(SLEEP_TIME_MILLIS);
				}
		    }
	    }
    }

   
	// clean up
    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bResp);
	UA_Variant_clear(&value);
	UA_Client_delete(client); //Disconnects the client internally 
	return EXIT_SUCCESS;
}