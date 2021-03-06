//en este archivo pongo cabeceras de funciones
// y nuevos tipos de datos como estructuras
//#ifndef packetsh
//#define packetsh

#include <linpal.h>
#define VER_TYPE 0x11


   struct TAG{
      unsigned char type[2];
      unsigned short length;
   };
   typedef struct TAG * LPTAG;

   struct TAG_ACNAME{
      unsigned short type;
      unsigned short length;
      unsigned char ac_name[8];
   };
   typedef struct TAG_ACNAME *acname;

   struct OPTION{
      unsigned char type;
      unsigned char length;  //longitud total incluidos los campos type length y data
      unsigned short data;
   };	
   typedef struct OPTION *lpcoption;
	
	
   struct NCP_COMPRESSION{
      unsigned char type;
      unsigned char length;
      unsigned short data;
      unsigned char maxSlotID;
      unsigned char compSlotID;
   };
   typedef struct NCP_COMPRESSION *ncpcompression;
	
	
   struct NCP_OPTION{
      unsigned char type;
      unsigned char length;
      //unsigned short
   };
   typedef struct NCP_OPTION * ncpoption;


   struct LPC_PACKET{
      unsigned char code;
      unsigned char identifier;
      unsigned short length;  //longitud total incluido los campos code, identifier, length y opciones

   };
   typedef struct LPC_PACKET * LPCPACKET;
	



   struct PPP_PACKET{
      unsigned char protocol[2];
      struct LPC_PACKET information;
   };
	


	
   struct PPPoED_PACKET{
      unsigned char ver_type;
      unsigned char code;
      unsigned short session_id;
      unsigned short length;  //longitud del payload (de los tags en este caso)
      struct TAG serviceName;
      struct TAG_ACNAME acname;
      struct TAG eol;
   };
   typedef struct PPPoED_PACKET * LPPPPDo3_FRAME;


   struct PPPoES_PACKET{
      unsigned char ver_type;
      unsigned char code;
      unsigned short session_id;
      unsigned short length;   //longitud del campo payload
      struct PPP_PACKET payload;
   };
   typedef struct PPPoES_PACKET * LPPPPSo3_FRAME;

   struct CLIENTE{
      unsigned short session_id;
      unsigned char DireccionDest[ETHERNET_ADDRESS_LENGTH];
      unsigned char estado;
   };
   typedef struct CLIENTE * cliente;

