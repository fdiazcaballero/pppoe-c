//en este archivo pongo cabeceras de funciones
// y nuevos tipos de datos como estructuras
//#ifndef packetsh
//#define packetsh

#include <linpal.h>
#define VER_TYPE 0x11

        
//estructura que usamos para dos tags en la fase de descubrimiento 
//el tag end-of-list y el tag service-name cuya longitud tambien es 0 para indicar
//que cualquier servicio es aceptable
   struct TAG{
      unsigned char type[2];
      unsigned short length;
   };
   typedef struct TAG * LPTAG;

//estructura que usamos para el tag ac-name en la fase de descubrimiento
   struct TAG_ACNAME{
      unsigned short type;
      unsigned short length;
      unsigned char ac_name[8];
   };
   typedef struct TAG_ACNAME *acname;

//estructura que usamos para la opcion de configuracion de lcp del MRU
   struct OPTION{
      unsigned char type;
      unsigned char length;  //longitud total incluidos los campos type length y data
      unsigned short data;
   };   
   typedef struct OPTION *lpcoption;
        
//estructura que usamos para la opcion de configuracion de IPCP del IP-Compression-Protocol
   struct NCP_COMPRESSION{
      unsigned char type;
      unsigned char length;
      unsigned short data;
      unsigned char maxSlotID;
      unsigned char compSlotID;
   };
   typedef struct NCP_COMPRESSION *ncpcompression;
        
//estructura que representa un paquete de la fase lcp, y que tambien usamos para
//los paquetes de la fase ncp, ya que el IPCP usa el mismo mecanismo de intercambio de 
//paquetes que el Link Control Protocol
//Por este motivo (que va a servir tanto para la fase lcp y la ncp) no se han incluido 
//las estructuras de las opciones de configuracion, al ser estas distintas en cada fase.
//Esto se hará posteriormente en el momento del envio de las tramas.
   struct LPC_PACKET{
      unsigned char code;
      unsigned char identifier;
      unsigned short length;  //longitud total incluido los campos code, identifier, length y opciones
   
   };
   typedef struct LPC_PACKET * LPCPACKET;
        


//estructura que representa un paquete ppp
   struct PPP_PACKET{
      unsigned char protocol[2];
        
        //trama lcp o ncp que va encapsulada dentro de la trama ppp
      struct LPC_PACKET information;
   };
        


//estructura que representa un paquete pppoe en la fase de descubrimiento
   struct PPPoED_PACKET{
      unsigned char ver_type;
      unsigned char code;
      unsigned short session_id;
      unsigned short length;  //longitud del payload (de los tags en este caso)
        //tags usados
      struct TAG serviceName;
      struct TAG_ACNAME acname;
      struct TAG eol;
   };
   typedef struct PPPoED_PACKET * LPPPPDo3_FRAME;

//estructura que representa un paquete pppoe en la fase de sesion
   struct PPPoES_PACKET{
      unsigned char ver_type;
      unsigned char code;
      unsigned short session_id;
      unsigned short length;   //longitud del campo payload
        //trama ppp encapsulada dentro del pppoes
      struct PPP_PACKET payload;
   };
   typedef struct PPPoES_PACKET * LPPPPSo3_FRAME;

//estructura que contiene los datos relevantes de los clientes que estan 
//siendo atendidos simultaneamente por un servidor, y que son necesarios para
//retomar la comunicacion en el punto en que se quedo, en el caso de existir
//otras comunicaciones de forma concurrente
   struct CLIENTE{
      unsigned short session_id;
      unsigned char DireccionDest[ETHERNET_ADDRESS_LENGTH];
      unsigned short estado;
   };
   typedef struct CLIENTE * cliente;


