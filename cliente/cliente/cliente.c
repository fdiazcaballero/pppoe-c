  /***************************************************************************
                            cliente.c  -  description
                               -------------------
      begin                : Wed Apr 6 2005
      copyright            : (C) 2005 by
      email                :
   ***************************************************************************/

  /***************************************************************************
   *                                                                         *
   *   This program is free software; you can redistribute it and/or modify  *
   *   it under the terms of the GNU General Public License as published by  *
   *   the Free Software Foundation; either version 2 of the License, or     *
   *   (at your option) any later version.                                   *
   *                                                                         *
   ***************************************************************************/

  #include <linpal.h>
  #include <stdio.h>
  #include "packets.h"
  #define lpc_send pppoes_send.payload.information
  #define lpc_rec pppoes_rec.payload.information
  #define lengthPPPoE sizeof(struct PPPoE_PACKET)

//variables que usamos
   int OK;
   int recibido;
   unsigned short protocolo;
   unsigned short ppp_protocol;
   unsigned short session_id=0;
   unsigned short bytesEnviados=0;
   char selec[MAX_NAME_LENGTH]="eth0";

   unsigned char identifier;
   unsigned char lpc_identifier;
   unsigned char dormir=0;
   unsigned char nombreac[]="unknown_";

//estructuras que usamos
   struct ADAPTER Adapter;
   
//una trama ethernet para envio y otra para recepcion
   struct ETHERNET_FRAME frame_send;
   struct ETHERNET_FRAME frame_rec;
   //una trama pppoe session para envio y otra para recepcion
   struct PPPoES_PACKET pppoes_rec;
   struct PPPoES_PACKET pppoes_send;
   //una trama pppoe de descubrimiento para envio y otra para recepcion
   struct PPPoED_PACKET pppoed_rec;
   struct PPPoED_PACKET pppoed_send;

//Opcion de configuracion de LCP   
   struct OPTION mru;
   //Opcion de configuracion de NCP
   struct NCP_COMPRESSION IPCompressProto;

//metodos
   void enviar_PADI(void);
   void recibir_PADO(void);
   void enviar_PADR(void);
   void recibir_PADS(void);
   void enviar_confreq_LCP(void);
   void enviar_confack_LCP(void);
   void enviar_confreq_NCP(void);
   void enviar_confack_NCP(void);
   void recibir_fasePPP(unsigned short , unsigned short);
   void enviar_terminate(unsigned char, unsigned char, unsigned char);
   void enviar_PADT(void);


    int main (void) {
   
   //rellenar los campos ver y type de pppoed y pppoes
      pppoed_send.ver_type = VER_TYPE;
      pppoes_send.ver_type = VER_TYPE;
   
   //rellenamos a 0 el adaptador
      memset(&Adapter,0,sizeof(struct ADAPTER));
   
   //abrimos el adaptador
      if ((NepalOpenAdapter(selec, &Adapter))==-1){
         printNep2Error("Error al abrir el adaptadotr");
         return -1;
      }
   
      //ponemos un filtro para solo permitir los paquetes cuya direccion destino sea broadcast o la nuestra
      if (NepalSetFilter(&Adapter, NULL, PACKET_TYPE_DIRECTED)==False){
         printNep2Error("Error NepalSetFilter");
         exit(1);
      }
     // INICIO DE LA FASE DE DESCUBRIMIENTO
     
     //dormimos el hilo durante 5 segundos para simular la lentitud en la maquina del cliente
      dormir=5;
      sleep(dormir);	
    // Envio por broadcast un paquete tipo PADI
      enviar_PADI();
   //espero a recibir un PADO
      recibir_PADO();
      sleep(1);
   
     //Envio del paquete PADR
      enviar_PADR();
   
     // Espero a recibir un paquete tipo PADS
      recibir_PADS();
      sleep(1);
   
      printf("Fase de descubrimiento concluida. Comienza la fase PPP\n");
   
     // INICIO DE LA SESIÓN PPP
     
     
     //Fase lcp
   
     // Envio de un confreq
      enviar_confreq_LCP();
     	
      recibir_fasePPP(0x2,0xc021);
   	
      recibir_fasePPP(0x1,0xc021);
   
      sleep(1);
        //envio de un confack
      enviar_confack_LCP();
   
      
      sleep(1);
   
      printf("Fase LCP concluida. Comienza la fase NCP\n");
      
   	//Fase NCP
   
      enviar_confreq_NCP();
   
      recibir_fasePPP(0x2,0x8021);
      
      recibir_fasePPP(0x1,0x8021);
      sleep(1);
     	
      enviar_confack_NCP();
   
     	
      
      sleep(1);
      
      printf("Fase NCP iniciada.\n");
      
   	
   	
   	//terminamos la fase ncp
      enviar_terminate(0x5,0x80,0x21);
     	
      recibir_fasePPP(0x6,0x8021);
   	
      recibir_fasePPP(0x5,0x8021);
   
      sleep(1);
        
      enviar_terminate(0x6,0x80,0x21);
      
      printf("Se ha cerrado la Fase NCP.\n");
   	
      sleep(1);
      
   	//terminamos la fase lcp
   	
      enviar_terminate(0x5,0xc0,0x21);
     	
      recibir_fasePPP(0x6,0xc021);
   	
      recibir_fasePPP(0x5,0xc021);
   
      sleep(1);
        
      enviar_terminate(0x6,0xc0,0x21);
      
      printf("Se ha cerrado la Fase LCP.\n");
   	
      sleep(1);
      
      enviar_PADT();
   
      sleep(1);
   
      
        //Se cierra el adaptador
      NepalCloseAdapter(&Adapter);
      return 0;
   }

    void enviar_PADI(void){
   
      //rellenamos los campos de la cabecera pppoed
      pppoed_send.code = 0x09;
      pppoed_send.session_id = session_id;
      pppoed_send.length = ntohs(sizeof(pppoed_send.eol)+sizeof(pppoed_send.acname)+sizeof(pppoed_send.serviceName));
      
   	//rellenamos los campos del tag service-name que se encuentra en el payload de la trama pppoed    
      pppoed_send.serviceName.type[0]=0x01;
      pppoed_send.serviceName.type[1]=0x01;
      pppoed_send.eol.length=0;  // Ponemos la longitud a 0, que indica que cualquier servicio es aceptable
   
   //rellenamos los campos del tag ac-name que se encuentra en el payload de la trama pppoed
      pppoed_send.acname.type=ntohs(0x0102);
      pppoed_send.acname.length=ntohs(8);
      memcpy(pppoed_send.acname.ac_name, nombreac,sizeof(pppoed_send.acname.ac_name));
   	
   	//rellenamos los campos del tag eol que se encuentra en el payload de la trama pppoed
      pppoed_send.eol.type[0]=0x00;
      pppoed_send.eol.type[1]=0x00;
      pppoed_send.eol.length=0;
   
   //ponemos la direccion destino como broadcast
      NepalEther2Byte("ff:ff:ff:ff:ff:ff", frame_send.DestinationAddress);
      
   	//direccion origen la nuestra
      memcpy(frame_send.SourceAddress,Adapter.CurrentAddress,ETHERNET_ADDRESS_LENGTH);
       
      memcpy(frame_send.Data, &pppoed_send, sizeof(pppoed_send));
   
      //protocolo correspondiente a pppoe fase de descubrimiento
      frame_send.Protocol[0]=0x88;
      frame_send.Protocol[1]=0x63;
      
   	
   	//comprobamos que los bytes que se van a enviar sean como minimo 60, si no lo
      //son, entonces enviamos 60
      if((ETHERNET_HEADER_LENGTH + sizeof(pppoed_send))>=60)
         bytesEnviados=ETHERNET_HEADER_LENGTH + sizeof(pppoed_send);
      else
         bytesEnviados=60;
   
   //enviamos la trama
      OK = NepalSendPacket(&Adapter, &frame_send, bytesEnviados);
      if (OK==-1){
         printNep2Error("Error al enviar un paquete PADI");
         exit(1);
      }
   }

    void recibir_PADO(void){
   
     // Espero a recibir un paquete tipo PADO
      recibido=0;
      while(recibido == 0){
         if(NepalReadPacket(&Adapter, &frame_rec, 1) > 0){
         //Comprobamos que el paquete vaya dirigido a nosotros
            if(memcmp(&(frame_rec.DestinationAddress), &(Adapter.CurrentAddress),6)==0){
               memcpy(&protocolo, &(frame_rec.Protocol[0]),2);
               protocolo=ntohs(protocolo);
               //vemos que sea de la fase de descubrimiento
               if(protocolo == 0x8863){
               //lo copiamos a una trama pppoed
                  memcpy(&pppoed_rec,&(frame_rec.Data[0]), sizeof(pppoed_rec));
                  //comprobamos que sea un pado
                  if(pppoed_rec.code== 0x07){
                  //Guardamos la dirección mac de nuestro servidor
                     memcpy(frame_send.DestinationAddress,frame_rec.SourceAddress,ETHERNET_ADDRESS_LENGTH);
                     
                  	//guardamos el nombre del servidor
                     memcpy(nombreac, pppoed_rec.acname.ac_name,sizeof( pppoed_rec.acname.ac_name ));
                  
                     recibido++;
                  }
               }
            }
         }
      }
   
   }

  	

    void enviar_PADR(void){
    
    //ponemos el codigo del padr
      pppoed_send.code = 0x19;
      
   	//ponemos el nombre del servidor
      memcpy(pppoed_send.acname.ac_name, nombreac, sizeof(pppoed_send.acname.ac_name));
   
      memcpy(frame_send.Data, &pppoed_send, sizeof(pppoed_send));
      
   	//comprobamos que los bytes que se van a enviar sean como minimo 60, si no lo
      //son, entonces enviamos 60
      if((ETHERNET_HEADER_LENGTH + sizeof(pppoed_send))>=60)
         bytesEnviados=ETHERNET_HEADER_LENGTH + sizeof(pppoed_send);
      else
         bytesEnviados=60;
   
   //enviamos el paquete
      OK = NepalSendPacket(&Adapter, &frame_send, bytesEnviados);
      if (OK==-1){
         printNep2Error("Error al enviar un paquete PADS");
         exit(1);
      }
   
   }

    void recibir_PADS(void){
   
      recibido=0;
      while(recibido == 0){
        //comprobar que realmente es un paquete de tipo PADO, si no lo es volver a recibir otro paquete
         if(NepalReadPacket(&Adapter, &frame_rec, 1) > 0){
         
         //Comprobamos que el paquete vaya dirigido a nosotros
            if(memcmp(&(frame_rec.DestinationAddress), &(Adapter.CurrentAddress),6)==0){
              
               memcpy(&protocolo, &(frame_rec.Protocol[0]),2);
               protocolo=ntohs(protocolo);
               
            	//vemos que sea de la fase de descubrimiento
               if(protocolo ==0x8863){
                  memcpy(&pppoed_rec,&(frame_rec.Data[0]), sizeof(pppoed_rec));
                  
               	//vemos que sea un paquete pads y que proceda de nuestro servidor
                  if(pppoed_rec.code== 0x65 && memcmp(pppoed_rec.acname.ac_name,nombreac, sizeof(pppoed_rec.acname.ac_name))==0){
                  
                  //guardamos el session identifier
                     session_id=pppoed_rec.session_id;
                  
                     recibido++;
                  }
               }
            }
         }
      }
   }


    void enviar_confreq_LCP(void){
   
   //rellenamos los campos de la nueva trama pppoe session
      pppoes_send.code=0x00;
      pppoes_send.session_id=session_id;
      
   	//ponemos el campo protocolo del ppp a 0xc021 (fase LCP)
      pppoes_send.payload.protocol[0]=0xc0;
      pppoes_send.payload.protocol[1]=0x21;
   
   
   //code del lcp a 0x1 (confreq)
      lpc_send.code=0x1;
      
   	//asignamos un identificador lcp
      lpc_identifier=(char)random();
   
      lpc_send.identifier=lpc_identifier;
   
   //asignamos los campos de la estructura mru
      mru.type=0x1;
      mru.length=0x04;
      mru.data=ntohs(1300);
      
   	//como mru es una configuration option de lcp pero no pertenece a esta estructura
   	//hay que sumar el tamaño de la opcion mru  mas el de la estructura lcp para el campo length de lcp
      lpc_send.length = ntohs(sizeof(lpc_send)+sizeof(mru));
      pppoes_send.length = ntohs(sizeof(pppoes_send.payload)+sizeof(mru));
   
     	//copiamos las estructuras al payload de la trama ethernet
      memcpy(frame_send.Data, &pppoes_send, sizeof(pppoes_send));
      memcpy(frame_send.Data+sizeof(pppoes_send),&mru,sizeof(mru));
   
     //protocolo pppoe fase de sesion ppp
      frame_send.Protocol[0]=0x88;
      frame_send.Protocol[1]=0x64;
      
   	//comprobamos que los bytes que se van a enviar sean como minimo 60, si no lo
      //son, entonces enviamos 60
      if((ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(mru))>=60)
         bytesEnviados=ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(mru);
      else
         bytesEnviados=60;
   
   //enviamos el paquete
      OK = NepalSendPacket(&Adapter, &frame_send,  bytesEnviados);
      if (OK==-1){
         printNep2Error("Error al enviar un paquete PADI");
         exit(1);
      }
   
   }

    void recibir_fasePPP(unsigned short code, unsigned short ppp_proto){
      recibido=0;
      while(recibido == 0){
        
         if(NepalReadPacket(&Adapter, &frame_rec, 1) > 0){
         
         //comprobamos que es un paquete enviado a este cliente
            if(memcmp(&(frame_rec.DestinationAddress), &(Adapter.CurrentAddress),6)==0){
               memcpy(&protocolo, &(frame_rec.Protocol[0]),2);
               protocolo=ntohs(protocolo);
               
               //vemos que es de la sesion ppp
               if(protocolo ==0x8864){
                  memcpy(&pppoes_rec, &(frame_rec.Data[0]), sizeof(pppoes_rec));
                  memcpy(&ppp_protocol, &(pppoes_rec.payload.protocol[0]),2);
                  ppp_protocol=ntohs(ppp_protocol);
                  
               	//vemos de que fase es (lcp o ncp)
                  if(pppoes_rec.code==0x00 && ppp_protocol==ppp_proto){
                  
                  //vemos que se trata de un confreq (o confack), que el identificador lcp no ha cambiado, y que corresponde con nuestra sesion
                     if((lpc_rec.code==code) && (lpc_identifier==lpc_rec.identifier) && (pppoes_rec.session_id==session_id) ){
                        recibido++;
                     
                     }
                  }
               }
            }
         }
      }
   }


    void enviar_confack_LCP(void){
        
   	  //codigo del confack
      lpc_send.code=0x2;
      lpc_send.identifier=lpc_identifier;
       
   	 //copiamos el mismo campo mru
      memcpy(&mru, frame_rec.Data+sizeof(pppoes_rec), sizeof(mru));
      lpc_send.length = ntohs(sizeof(lpc_send)+sizeof(mru));
      memcpy(frame_send.Data, &pppoes_send, sizeof(pppoes_send));
      memcpy(frame_send.Data+sizeof(pppoes_send), &mru, sizeof(mru));
   	  
   	//comprobamos que los bytes que se van a enviar sean como minimo 60, si no lo
      //son, entonces enviamos 60
      if((ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(mru))>=60)
         bytesEnviados=ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(mru);
      else
         bytesEnviados=60;
   
   //enviamos el paquete
      OK = NepalSendPacket(&Adapter, &frame_send,  bytesEnviados);
      if (OK==-1){
         printNep2Error("Error al enviar un paquete PADI");
         exit(1);
      }
   }


    void enviar_confreq_NCP(void){
    
    //damos valores a los campos de la Opcion de configuracion IP-Compression-Protocol
      IPCompressProto.type=0x02;
      IPCompressProto.length=0x06;
      IPCompressProto.data=ntohs(0x002d);
      IPCompressProto.maxSlotID=15;
      IPCompressProto.compSlotID=0;
   
      pppoes_send.code=0x00;
      pppoes_send.session_id=session_id;
      
   	//campo protocolo de ppp a 0x8021 (fase ncp)
      pppoes_send.payload.protocol[0]=0x80;
      pppoes_send.payload.protocol[1]=0x21;
   
   
   //seguimos usando una estructura lcp, ya que el IPCP usa el mismo mecanismo de intercambio de 
   //paquetes que el Link Control Protocol
   
   //confreq
      lpc_send.code=0x1;
   
   
      lpc_send.identifier=lpc_identifier;
   
   
      lpc_send.length = ntohs(sizeof(lpc_send)+sizeof(IPCompressProto));
      pppoes_send.length = ntohs(sizeof(pppoes_send.payload)+sizeof(IPCompressProto));
   
     	
      memcpy(frame_send.Data, &pppoes_send, sizeof(pppoes_send));
      memcpy(frame_send.Data+sizeof(pppoes_send),&IPCompressProto,sizeof(IPCompressProto));
   
   //campo protocolo fase de sesion ppp
      frame_send.Protocol[0]=0x88;
      frame_send.Protocol[1]=0x64;
      
   	//comprobamos que los bytes que se van a enviar sean como minimo 60, si no lo
      //son, entonces enviamos 60
      if((ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(IPCompressProto))>=60)
         bytesEnviados=ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(IPCompressProto);
      else
         bytesEnviados=60;
   
   //enviamos el paquete
      OK = NepalSendPacket(&Adapter, &frame_send,  bytesEnviados);
      if (OK==-1){
         printNep2Error("Error al enviar un paquete confreq de ncp");
         exit(1);
      }
   
   }

    void enviar_confack_NCP(void){
   
   //confack
      lpc_send.code=0x2;
      lpc_send.identifier=lpc_identifier;
     
   //copiamos la opcion IP-Compression-protocol que hemos recibido para enviar nosotros la misma
      memcpy(&IPCompressProto, frame_rec.Data+sizeof(pppoes_rec), sizeof(IPCompressProto));
      lpc_send.length = ntohs(sizeof(lpc_send)+sizeof(IPCompressProto));
      pppoes_send.length = ntohs(sizeof(pppoes_send.payload)+sizeof(IPCompressProto));
      memcpy(frame_send.Data, &pppoes_send, sizeof(pppoes_send));
      memcpy(frame_send.Data+sizeof(pppoes_send), &IPCompressProto, sizeof(IPCompressProto));
   
   	//comprobamos que los bytes que se van a enviar sean como minimo 60, si no lo
      //son, entonces enviamos 60
      if((ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(IPCompressProto))>=60)
         bytesEnviados=ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(IPCompressProto);
      else
         bytesEnviados=60;
   
   //enviamos el paquete
      OK = NepalSendPacket(&Adapter, &frame_send,  bytesEnviados);
      if (OK==-1){
         printNep2Error("Error al enviar un paquete PADI");
         exit(1);
      }
   }
   
    void enviar_terminate(unsigned char code, unsigned char ppp_proto0, unsigned char ppp_proto1){
   
   //rellenamos los campos de la nueva trama pppoe session
      pppoes_send.code=0x00;
      pppoes_send.session_id=session_id;
      
   	//ponemos el campo protocolo del ppp a 0xc021 (fase LCP) o a 0x8021 (fase NCP)
      pppoes_send.payload.protocol[0]=ppp_proto0;
      pppoes_send.payload.protocol[1]=ppp_proto1;
   
   //0x5 -> terminate-request   ó     0x6 -> terminate-ack
      lpc_send.code=code;
      
      lpc_send.identifier=lpc_identifier;
   
      lpc_send.length = ntohs(sizeof(lpc_send));
      pppoes_send.length = ntohs(sizeof(pppoes_send.payload));
   
     	//copiamos las estructuras al payload de la trama ethernet
      memcpy(frame_send.Data, &pppoes_send, sizeof(pppoes_send));
      
   
     //protocolo pppoe fase de sesion ppp
      frame_send.Protocol[0]=0x88;
      frame_send.Protocol[1]=0x64;
      
   	//comprobamos que los bytes que se van a enviar sean como minimo 60, si no lo
      //son, entonces enviamos 60
      if((ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(mru))>=60)
         bytesEnviados=ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(mru);
      else
         bytesEnviados=60;
   
   //enviamos el paquete
      OK = NepalSendPacket(&Adapter, &frame_send,  bytesEnviados);
      if (OK==-1){
         printNep2Error("Error al enviar un paquete PADI");
         exit(1);
      }
   
   
   }
   
    void enviar_PADT(void){
   
   //protocolo pppoe fase de sesion ppp
      frame_send.Protocol[0]=0x88;
      frame_send.Protocol[1]=0x63;
      
      pppoed_send.code=0xa7;
      pppoed_send.length=0;
      pppoed_send.session_id=session_id;
   	
      memcpy(frame_send.Data,&pppoed_send,6);
   	
      	//comprobamos que los bytes que se van a enviar sean como minimo 60, si no lo
      //son, entonces enviamos 60
      if((ETHERNET_HEADER_LENGTH+6)>=60)
         bytesEnviados=ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(mru);
      else
         bytesEnviados=60;
         
   		//enviamos el paquete
      OK = NepalSendPacket(&Adapter, &frame_send,  bytesEnviados);
      if (OK==-1){
         printNep2Error("Error al enviar un paquete PADI");
         exit(1);
      }
   
         
   }
   
   	
   
	
	


