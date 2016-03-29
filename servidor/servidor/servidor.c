 /***************************************************************************
                           servidor.c  -  description
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

//variables usadas
   int OK;
   int recibido;
   //indice para añadir nuevas entradas en el array de clientes
   unsigned short int indice=-1;
   //indice para buscar clientes en el array de clientes por el session_id
   unsigned short int indice2=0;
   //indice para buscar clientes en el array de clientes por la direccion MAC del cliente
	//y que es usado cuando aun no se ha asignado un identificador de sesion a la comunicacion
   unsigned short int indice3=0;
   //array que contendra la direccion de broadcast
   unsigned char broadd[6];
   unsigned short bytesEnviados=0;
   //nombre del servidor
   unsigned char nombreac[]="kokkonia";



   unsigned char finDesc=-1;  //cuando sea 0 indica que se ha acabado la fase de descubrimiento
   unsigned char finLCP=-1;  //cuando sea 0 indica que se ha acabado la fase lcp
   unsigned char finNCP=-1;
   unsigned char LCPcerrado=-1;
   unsigned char NCPcerrado=-1;

 	//usaremos la ethernet1
   char selec[MAX_NAME_LENGTH]="eth0";
   unsigned short int protocolo;
   unsigned short int ppp_protocol;
   unsigned char lpc_identifier=0;

//estructuras que se van a usar
   struct ADAPTER Adapter;
   struct CLIENTE clientes[10];
   struct CLIENTE clientesAux[10];
   
	//una trama ethernet para envio y otra para recepcion
   struct ETHERNET_FRAME frame_send;
   struct ETHERNET_FRAME frame_rec;
   
	//una trama pppoe session para envio y otra para recepcion
   struct PPPoES_PACKET pppoes_rec;
   struct PPPoES_PACKET pppoes_send;
   
	//una trama pppoe de descubrimiento para envio y otra para recepcion
   struct PPPoED_PACKET pppoed_rec;
   struct PPPoED_PACKET pppoed_send;
   
	//opcion de la fase lcp
   struct OPTION mru;
   
	//opcion de la fase ncp
   struct NCP_COMPRESSION IPCompressProto;
   
   struct CLIENTE cli;
	
	
	//metodos
   void enviar_PADO(void);
   void enviar_PADS(void);
   void enviar_conf_LCP(unsigned char);
   int buscarIdentifier(unsigned short);
   void enviar_conf_NCP(unsigned char);
   int buscarDirDest(void);
   void eliminarCliente(void);
   void enviar_terminate(unsigned char, unsigned char, unsigned char);


    int main (void) {
   
   //direccion de broadcast
      broadd[0]=255;
      broadd[1]=255;
      broadd[2]=255;
      broadd[3]=255;
      broadd[4]=255;
      broadd[5]=255;     
          	
   
      memset(&Adapter,0,sizeof(struct ADAPTER));  //rellenar a 0 Adapter antes de empezar a trabajar con él
      memset(clientes,0,sizeof(clientes));      //rellenar a 0 clientes antes de empezar a trabajar con él
   
    	
    //Abrir el adaptador
      if ((NepalOpenAdapter(selec, &Adapter))==-1){
         printNep2Error("Error al abrir el adaptador");
         return -1;
      }
   
   //ponemos un filtro para solo permitir los paquetes cuya direccion destino sea broadcast o la nuestra
      if ((NepalSetFilter(&Adapter, NULL, OR_DIRECTED_BROADCAST))== False){
         printNep2Error("Error NepalSetFilter");
         exit(1);
      }
   
       
     //bucle que se ejecuta continuamente a la espera de recibir nuevas tramas
      while(1){
      
      //rellenemos a 0 las estructuras
         memset(&frame_rec,0,sizeof(frame_rec));
         memset(&lpc_rec,0,sizeof(lpc_rec));
         
         if(NepalReadPacket(&Adapter, &frame_rec, 0) > 0){
         
            memcpy(&protocolo, &(frame_rec.Protocol[0]),2);
            protocolo=ntohs(protocolo);
            
         	
            // INICIO DE LA FASE DE DESCUBRIMIENTO
         
         //comprobamos que el paquete pertece a la fase de descubrimiento
            if(protocolo == 0x8863){
            
            //comprobamos que la direccion destino del paquete sea la de broadcast o la nuestra por si el filtro no funciona
               if((memcmp(&(frame_rec.DestinationAddress),broadd ,6)==0) || (memcmp(&(frame_rec.DestinationAddress),&(Adapter.CurrentAddress),6)==0)){
               
               //copiamos el campo data de la trama ethernet en una estructura pppoed
                  memcpy(&pppoed_rec, &(frame_rec.Data[0]), sizeof(pppoed_rec));
                  
               	//comprobar que es un paquete de tipo PADI
                  if(pppoed_rec.code== 0x09){
                  
                     enviar_PADO();
                  }
                  
                  //si no vemos si es un PADR y comprabamos que vaya dirigido a este servidor por medio del nombre del servidor
                  else if(pppoed_rec.code == 0x19 && memcmp(pppoed_rec.acname.ac_name,nombreac,sizeof(pppoed_rec.acname.ac_name))==0){
                     
                  	//buscamos este cliente en nuestro array de clientes y si esta comprobamos que 
                  	//realmente se encuentra en este estado (el metodo buscarDirDest devuelve en indice3 la posicion 
                  	//donde se encuentra)
                     if(buscarDirDest()>-1 && clientes[indice3].estado==0){
                        enviar_PADS();
                        finDesc=1; //fin de la fase de desc
                        clientes[indice3].estado=1;  //incrementamos el estado
                     }
                  }

                   //vemos si recibimos un PADT
                  else if(pppoed_rec.code == 0xa7){
                     
                  	//buscamos este cliente en nuestro array de clientes y si esta comprobamos que 
                  	//realmente se encuentra en este estado (el metodo buscarDirDest devuelve en indice3 la posicion 
                  	//donde se encuentra)
                     if(buscarDirDest()>-1 && clientes[indice3].estado==10 && LCPcerrado){
                        
                        clientes[indice3].estado=11;  //incrementamos el estado
                        eliminarCliente();
                     }
                  }
               }
}
            
            
            
            
            //FASE LCP
               if(protocolo == 0x8864){
               
               //comprobamos que la direccion destino del paquete sea la de broadcast o la nuestra por si el filtro no funciona
               
                  if((memcmp(&(frame_rec.DestinationAddress),broadd ,6)==0) || (memcmp(&(frame_rec.DestinationAddress),&(Adapter.CurrentAddress),6)==0)){
                     memcpy(&pppoes_rec, &(frame_rec.Data[0]), sizeof(pppoes_rec));
                     memcpy(&ppp_protocol, &(pppoes_rec.payload.protocol[0]),2);
                     ppp_protocol=ntohs(ppp_protocol);
                      printf("oooo\n");
                  //comprobamos que estamos en la fase de sesion, de lcp, que el cliente se encuentra en nuestro array y que se 
                  //ha acabado la fase de descubrimiento
                     if(pppoes_rec.code==0x00 && ppp_protocol==0xc021 && buscarIdentifier(pppoes_rec.session_id)>-1 && finDesc){
                          printf("oooo\n");
                     //si se ha recibido un confreq y el estado es el correcto
                        if(lpc_rec.code==0x1 && clientes[indice2].estado==1 ){
                        
                        //se incrementa el estado
                           clientes[indice2].estado=2;
                        
                        //se guarda el identificador lcp
                           lpc_identifier=lpc_rec.identifier;
                        
                        //se envia un confack LCP al cliente como respuesta a su confreq
                           printf("oooo\n");
                           enviar_conf_LCP(0x2);
                        
                        //enviamos un confreq al cliente
                           enviar_conf_LCP(0x1);
                        
                        }
                        
                        //miramos si lo que se ha recibido en un confack y comprobamos el estado
                        else if( lpc_rec.code==0x2 && clientes[indice2].estado==2){
                           clientes[indice2].estado=3;
                        
                           finLCP=1;  //fin de la fase lcp
                        }
                        
                        else if(lpc_rec.code==0x5 && clientes[indice2].estado==8&&NCPcerrado){
                           clientes[indice2].estado=9;
                        
                        //se envia un terminate-ack LCP al cliente como respuesta a su confreq
                           enviar_terminate(0x6,0xc0,0x21);
                        //se envia un terminate-req LCP al cliente
                           enviar_terminate(0x5,0xc0,0x21);
                        }
                        
                        else if( lpc_rec.code==0x6 && clientes[indice2].estado==9&&finNCP) {
                           clientes[indice2].estado=10;
                           LCPcerrado=1;
                        }
                     }
                     
                     //FASE NCP
                     
                     //comprobamos que estamos en la fase de sesion, de ncp, que el cliente se encuentra en nuestro array y que se 
                     //ha acabado la fase de lcp
                     else if(pppoes_rec.code==0x00 && ppp_protocol==0x8021 && buscarIdentifier(pppoes_rec.session_id)>-1&& finLCP){
                     //si se ha recibido un confreq y el estado es el correcto
                        if(lpc_rec.code==0x1 && clientes[indice2].estado==3){
                           clientes[indice2].estado=4;
                        
                        //se envia un confack NCP al cliente como respuesta a su confreq
                           enviar_conf_NCP(0x2);
                        //se envia un confreq NCP al cliente
                           enviar_conf_NCP(0x1);
                        }
                        
                        //miramos si lo que se ha recibido en un confack y comprobamos el estado
                        else if( lpc_rec.code==0x2 && clientes[indice2].estado==4) {
                           clientes[indice2].estado=6;
                           finNCP=1;
                        }
                        
                        else if(lpc_rec.code==0x5 && clientes[indice2].estado==6&&finNCP){
                           clientes[indice2].estado=7;
                        
                        //se envia un terminate-ack NCP al cliente como respuesta a su confreq
                           enviar_terminate(0x6,0x80,0x21);
                        //se envia un terminate-req NCP al cliente
                           enviar_terminate(0x5,0x80,0x21);
                        }
                        
                        else if( lpc_rec.code==0x6 && clientes[indice2].estado==7&&finNCP) {
                           clientes[indice2].estado=8;
                           NCPcerrado=1;
                        }
                     
                     }
                  }

            }
         
         }
      }
   
   
   
    // Cerramos el adaptador
      NepalCloseAdapter(&Adapter);
      printf("esto es todo\n");
      return 0;
   }

    void enviar_PADO(void){
    //Envio de un paquete de tipo PADO
      indice=indice+1;
      
   	//comprobamos que el numero de clientes no supere 10
      if(indice<10){
      
      //rellenamos los campos de la cabecera pppoed
         pppoed_send.ver_type = VER_TYPE;
         pppoed_send.code = 0x07;
         
      	//rellenamos los campos del tag eol que se encuentra en el payload de la trama pppoed
         pppoed_send.eol.type[0]=0x00;
         pppoed_send.eol.type[1]=0x00;
         pppoed_send.eol.length=0;
      
          //rellenamos los campos del tag service-name que se encuentra en el payload de la trama pppoed    
         pppoed_send.serviceName.type[0]=0x01;
         pppoed_send.serviceName.type[1]=0x01;
         pppoed_send.eol.length=0;  // Ponemos la longitud a 0, que indica que cualquier servicio es aceptable
      
      	//rellenamos los campos del tag ac-name que se encuentra en el payload de la trama pppoed
         pppoed_send.acname.type=ntohs(0x0102);
         pppoed_send.acname.length=ntohs(8);
         
      	//le ponemos el nombre de este servidor
         memcpy(pppoed_send.acname.ac_name, nombreac,sizeof(pppoed_send.acname.ac_name));
      
         pppoed_send.session_id=clientes[indice].session_id;
      
      //asignemos la longitud del payload del paquete pppoed, que es la suma de los tres tags
         pppoed_send.length = ntohs(sizeof(pppoed_send.eol)+sizeof(pppoed_send.acname)+sizeof(pppoed_send.serviceName));
      
      //copiamos la direccion de destino
         memcpy(frame_send.DestinationAddress,frame_rec.SourceAddress,ETHERNET_ADDRESS_LENGTH);
         
      	//creamos una nueva entrada de un nuevo cliente. Para ello guardamos la direccion MAC
      	//del cliente en el campo direccionDest de la estructura cliente correspondiente del array de clientes
         memcpy(clientes[indice].DireccionDest,frame_rec.SourceAddress,ETHERNET_ADDRESS_LENGTH);
      
      //asignamos la direccion origen de la trama a enviar (la nuestra)
         memcpy(frame_send.SourceAddress,Adapter.CurrentAddress,ETHERNET_ADDRESS_LENGTH);
         
      	//copiamos la trama pppoed en el payload de la trama ethernet
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
            
      /* Enviar un paquete de tipo PADO al host*/
         OK = NepalSendPacket(&Adapter, &frame_send,bytesEnviados);
         if (OK==-1){
            printNep2Error("Error al enviar un paquete PADO");
            exit(1);
         }
      }
      else{
         printNep2Error("Error: el servidor no puede atender a mas de 10 clientes");
         exit(1);
      }
   }


    void enviar_PADS(void){
    /* Enviar un paquete de tipo PADS al servidor elegido */
      if(indice<10){
      
      //asignamos un identificador de session a este cliente de la tabla de clientes
      //recordamos que indice3 ha sido actuaclizado por el metodo de busqueda que previamente se ha
      //llamado
         clientes[indice3].session_id=(short)random();
         
         pppoed_send.ver_type = VER_TYPE;
         pppoed_send.code = 0x65;
         pppoed_send.session_id=clientes[indice3].session_id;
         
         pppoed_send.length = ntohs(sizeof(pppoed_send.eol)+sizeof(pppoed_send.acname)+sizeof(pppoed_send.serviceName));
      
         memcpy(frame_send.DestinationAddress,frame_rec.SourceAddress,ETHERNET_ADDRESS_LENGTH);
         memcpy(frame_send.SourceAddress,Adapter.CurrentAddress,ETHERNET_ADDRESS_LENGTH);
         
      	//copiamos la trama pppoed en el payload de la trama ethernet
         memcpy(frame_send.Data, &pppoed_send, sizeof(pppoed_send));
      
      	//comprobamos que los bytes que se van a enviar sean como minimo 60, si no lo
      	//son, entonces enviamos 60
         if((ETHERNET_HEADER_LENGTH + sizeof(pppoed_send))>=60)
            bytesEnviados=ETHERNET_HEADER_LENGTH + sizeof(pppoed_send);
         else
            bytesEnviados=60;
      
      
         OK = NepalSendPacket(&Adapter, &frame_send, bytesEnviados);
         if (OK==-1){
            printNep2Error("Error al enviar un paquete PADS");
            exit(1);
         }
      }
      else{
         printNep2Error("Error: el servidor no puede atender a mas de 10 clientes");
         exit(1);
      }
   }

 	//con el parametro code se determina si lo que se envia es un confreq (0x1) o un confack (0x2)
    void enviar_conf_LCP(unsigned char code){
      pppoes_send.ver_type = VER_TYPE;
      
   	//se establece la session_id a la nueva trama pppoe de fase de sesion
   	//indice2 ha sido actualizado por el metodo que busca el cliente por la 
   	//identificador se sesion que ha sido llamado previamente
      pppoes_send.session_id=clientes[indice2].session_id;
      pppoes_send.code=0x00;
      
   	//ponemos el campo protocolo de la trama ppp a 0xc021 que indica que estamos en la fase LCP
      pppoes_send.payload.protocol[0]=0xc0;
      pppoes_send.payload.protocol[1]=0x21;
      
   	
      lpc_send.code=code;
      //asignamos el identificador LCP
      lpc_send.identifier=lpc_identifier;
   
      memcpy(frame_send.DestinationAddress,frame_rec.SourceAddress,ETHERNET_ADDRESS_LENGTH);
   
   //copiamos la misma mru que nos ha enviado el cliente en su confreq
      memcpy(&mru, frame_rec.Data+sizeof(pppoes_rec), sizeof(mru));
      
   	//asignamos la longitud de la trama lcp, como la estructura mru no pertenece a la estructura LCP
   	//hay que sumarle su longitud
      lpc_send.length = ntohs(sizeof(lpc_send)+sizeof(mru));
      pppoes_send.length = ntohs(sizeof(pppoes_send.payload)+sizeof(mru));
   
   //copiamos las estructuras en el payload de la trama ethernet
      memcpy(frame_send.Data, &pppoes_send, sizeof(pppoes_send));
      memcpy(frame_send.Data+sizeof(pppoes_send), &mru, sizeof(mru));
   
   
      //protocolo de pppoe de la fase de sesion
      frame_send.Protocol[0]=0x88;
      frame_send.Protocol[1]=0x64;
      
   		//comprobamos que los bytes que se van a enviar sean como minimo 60, si no lo
      	//son, entonces enviamos 60      
      if((ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(mru))>=60)
         bytesEnviados=ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(mru);
      else
         bytesEnviados=60;
   
   
      OK = NepalSendPacket(&Adapter, &frame_send, bytesEnviados);
      if (OK==-1){
         printNep2Error("Error al enviar un paquete confreq");
         exit(1);
      }
   
   }



//con el parametro code se determina si lo que se envia es un confreq (0x1) o un confack (0x2)
    void enviar_conf_NCP(unsigned char code){
      pppoes_send.ver_type = VER_TYPE;
      //establecemos el identificador de sesion correspondiente a este cliente
      pppoes_send.session_id=clientes[indice2].session_id;
      pppoes_send.code=0x00;
      //ponemos el campo protocolo de la trama ppp a 0xc021 que indica que estamos en la fase NCP
      pppoes_send.payload.protocol[0]=0x80;
      pppoes_send.payload.protocol[1]=0x21;
      
   //seguimos usando una estructura lcp, ya que el IPCP usa el mismo mecanismo de intercambio de 
   //paquetes que el Link Control Protocol
   
      lpc_send.code=code;
      lpc_send.identifier=lpc_identifier;
      
   	//copiamos la misma Protocolo de compresion IP que nos ha enviado el cliente en su confreq
      memcpy(&IPCompressProto, frame_rec.Data+sizeof(pppoes_rec), sizeof(IPCompressProto));
   
      memcpy(frame_send.DestinationAddress,frame_rec.SourceAddress,ETHERNET_ADDRESS_LENGTH);
                  
      lpc_send.length = ntohs(sizeof(lpc_send)+sizeof(IPCompressProto));
      pppoes_send.length = ntohs(sizeof(pppoes_send.payload)+sizeof(IPCompressProto));
   
      memcpy(frame_send.Data, &pppoes_send, sizeof(pppoes_send));
      memcpy(frame_send.Data+sizeof(pppoes_send), &IPCompressProto, sizeof(IPCompressProto));
   
   
       //protocolo de pppoe de la fase de sesion
      frame_send.Protocol[0]=0x88;
      frame_send.Protocol[1]=0x64;
      
   		//comprobamos que los bytes que se van a enviar sean como minimo 60, si no lo
      	//son, entonces enviamos 60 
      if((ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(IPCompressProto))>=60)
         bytesEnviados=ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(IPCompressProto);
      else
         bytesEnviados=60;
   
   
      OK = NepalSendPacket(&Adapter, &frame_send, ETHERNET_HEADER_LENGTH+sizeof(pppoes_send)+sizeof(IPCompressProto));
      if (OK==-1){
         printNep2Error("Error al enviar un paquete confreq");
         exit(1);
      }
   
   }
   
    void enviar_terminate(unsigned char code, unsigned char ppp_proto0, unsigned char ppp_proto1){
   
   //rellenamos los campos de la nueva trama pppoe session
      pppoes_send.code=0x00;
       //establecemos el identificador de sesion correspondiente a este cliente
      pppoes_send.session_id=clientes[indice2].session_id;
      
      
   	//ponemos el campo protocolo del ppp a 0xc021 (fase LCP) o a 0x8021 (fase NCP)
      pppoes_send.payload.protocol[0]=ppp_proto0;
      pppoes_send.payload.protocol[1]=ppp_proto1;
   
   //0x5 -> terminate-request   ó     0x6 -> terminate-ack
      lpc_send.code=code;
      
      lpc_send.identifier=lpc_identifier;
   
      lpc_send.length = ntohs(sizeof(lpc_send));
      pppoes_send.length = ntohs(sizeof(pppoes_send.payload));
      
      memcpy(frame_send.DestinationAddress,frame_rec.SourceAddress,ETHERNET_ADDRESS_LENGTH);
   
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
         printNep2Error("Error al enviar un paquete terminate");
         exit(1);
      }
   
   
   }
   
   

//metodo que busca un cliente en el array de clientes por su identificador de sesion
    int buscarIdentifier(unsigned short iden){
   
      indice2=0;
      while((indice2<10)&&((clientes[indice2].session_id)!=iden)){
         indice2=indice2+1;
      }
   
   //si lo ha encontrado (el indice es menor que 10) se devuelve indice2
      if (indice2<10)
         return indice2;
      else
         return -1;
   }

//metodo que busca un cliente en el array de clientes por su direccion MAC
    int buscarDirDest(void){
   
      indice3=0;
      while((indice3<10)&&(memcmp(&(clientes[indice3].DireccionDest),&(frame_rec.SourceAddress),sizeof(frame_rec.SourceAddress))!=0)){
         indice3=indice3+1;
      }
   
   //si lo ha encontrado (el indice es menor que 10) se devuelve indice3
      if (indice3<10)
         return indice3;
      else
         return -1;
   }
   
	
    void eliminarCliente(void){

      memset(clientesAux,0,10*sizeof(cli));
      memset(&clientes[indice3],0,sizeof(cli));
   
      memcpy(clientesAux,&(clientes[indice3+1]),(indice-indice3)*sizeof(cli));
   
      memcpy(&(clientes[indice3]),clientesAux,(indice-indice3)*sizeof(cli));
      memset(&clientes[indice],0,sizeof(cli));
   
      indice=indice-1;
   
   }

