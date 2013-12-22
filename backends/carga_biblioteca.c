/*
 * Maimonides Suite v1.0.11 - advertisment/information player via embedded Raspberry Pi
 *
 *  Copyright 2012-2013 by it's authors. 
 *
 *  Some rights reserved. See COPYING, AUTHORS.
 *  This file may be used under the terms of the GNU General Public
 *  License version 3.0 as published by the Free Software Foundation
 *  and appearing in the file COPYING included in the packaging of
 *  this file.
 *
 *  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 *  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <gtk/gtk.h>
#include <cairo.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "carga_biblioteca.h"
#include "carga_imagenes.h"
#include "carga_audio.h"
#include "carga_video.h"
#include "../common/pi_xml_parser.h"

#ifdef linux
int return_app_path (gchar **path)
{
//cleaning
  // gchar *ppath_quotes = NULL;
  gchar *ppath_noquotes = NULL,*parsestring;
  gchar buffer[200];
  int length;

  length = readlink("/proc/self/exe", buffer, sizeof(buffer));
  buffer[length] = '\0';

  if (length < 0) {
    printf("ERROR: resolving symlink /proc/self/exe.");
    return(-1);
  }
  if (length >= sizeof(buffer)) {
    fprintf(stderr, "ERROR: Path too long.\n");
    return(-1);
  }  
      
  //ppath_quotes = malloc((length+15)*sizeof(gchar));
  ppath_noquotes = malloc((length+25)*sizeof(gchar));

  //strcpy(ppath_quotes,fullpath);
  strcpy(ppath_noquotes,buffer);
  /*  
  for (j = length;j>=0;j--)
    {	
      if(j!=0)
	ppath_quotes[j] = ppath_quotes[j-1];
      if(j==0)
	ppath_quotes[j] = '"';
    }
  */
  parsestring = strstr (ppath_noquotes,appname);
  strcpy(parsestring,"");
  /*  parsestring = strstr (ppath_quotes,appname);
  strcpy(parsestring,"");
  */

  /*   if (ppath_quotes != NULL)
       free(ppath_quotes);*/
  *path = ppath_noquotes;
 
  return 0;
   
}


int carga_usb() 
{
  int ret;
  int childExitStatus;
  pid_t pid;
  int status;
  gchar* dest = "./";
  gchar* command;
  command = malloc(strlen(usbpath)*sizeof(gchar)+64*sizeof(gchar));
  sprintf(command,"ls %s/config.xml",usbpath);
  ret = system(command);
  if(ret >0) //fallo cargando desde USB ret =512 ó 256
    return (ret);
  sprintf(command,"rm -f ./imagenes/* ./musica/* ./video/ ./config.xml");
  ret = system(command);

  sprintf(command,"cp %s/imagenes/* ./imagenes/",usbpath);
  ret = system(command);

  sprintf(command,"cp %s/musica/* ./musica/",usbpath);
  ret = system(command);

  sprintf(command,"cp %s/video/* ./video/",usbpath);
  ret = system(command);

  sprintf(command,"cp %s/config.xml ./",usbpath);
  ret = system(command);

  //free(dest);
  free(command);
  return(ret);
}
int guarda_usb() 
{
  int ret;
  gchar* dest = "./";
  gchar* command;
  command = malloc(strlen(usbpath)*sizeof(gchar)+64*sizeof(gchar));
  printf("INFO: Loading to usb\n");
  sprintf(command,"cp -r ./imagenes ./musica ./video ./config.xml %s",usbpath);
  //pid = fork();
    //if (pid == 0) { /* child */
 
  ret = system(command);
  //free(dest);
  free(command);
  return(ret);
}
#endif

int cargar_XML_biblioteca (int MODE, struct imagen** image_index,
			   struct pista** audios_index,struct pista** videos_index,struct glob_settings** settings)
{
  struct imagen* image_index2;
  struct pista* audios_index2, *videos_index2;
  struct glob_settings* settings2;
  int ret=0;

  image_index2 = *image_index;
  audios_index2 = *audios_index;
  videos_index2 = *videos_index;
  settings2 = *settings;
  printf("INFO: carga XML biblioteca!\n");
  switch(MODE)
    {
    case(FILES_MODE):
      ret = populateDoc("config.xml",&image_index2,&audios_index2,&videos_index2,&settings2);
      break;
    case(XML_MODE):
      ret = recoverDoc("config.xml",&image_index2,&audios_index2,&videos_index2,&settings2);
      break;
    default:
      return -1;
      break;
    }
  *image_index = image_index2;
  *audios_index = audios_index2;
  *videos_index = videos_index2;
  *settings = settings2;
  printf("INFO: fin carga XML biblioteca!\n");
  return ret;
}
int cargar_recursos_biblioteca (int MODE,struct imagen** image_index
				 ,struct pista** audio_index,struct pista** video_index,struct glob_settings** settings)
{
  int ret=0,k;
  struct imagen* image_index2;
  struct pista* audios_index2, *videos_index2;
  struct glob_settings* settings2;

  image_index2 = *image_index;
  audios_index2 = *audio_index;
  videos_index2 = *video_index;
  settings2 = *settings;
  printf("INFO: carga_biblioteca!\n");
  //recover from usb
 
  if (MODE == FILES_MODE)
    {
      ret = carga_imagenes(MODE,&image_index2);
      printf("MEJORAR: retorno de la carga de imagenes = %d \n",ret);
      for(k=0;k<no_images;k++)
	printf("DEBUG: funcion cargar_recursos_biblioteca , imagen %d, nombre %s \n",k+1,image_index2[k].name);
      //errores?
      ret = carga_audio(MODE,&audios_index2);
      printf("MEJORAR: retorno de la carga de pistas = %d \n",ret);
      for(k=0;k<no_pistas;k++)
	printf("DEBUG: funcion cargar_recursos_biblioteca , pista %d, nombre %s \n",k+1,audios_index2[k].name);
      //errores?
      ret = carga_video(MODE,&videos_index2);
      printf("MEJORAR: retorno de la carga de videos = %d \n",ret);
       for(k=0;k<no_videos;k++)
	printf("DEBUG: funcion cargar_recursos_biblioteca , pista %d, nombre %s \n",k+1,videos_index2[k].name);
      ret = cargar_XML_biblioteca (MODE, &image_index2,&audios_index2,&videos_index2,&settings2);
      g_printf("MEJORAR: retorno de la escritura del XML = %d \n",ret);
    }
  else if(MODE ==XML_MODE)
    {
      ret = cargar_XML_biblioteca (MODE, &image_index2,&audios_index2,&videos_index2,&settings2);
    }
   else if(MODE ==SAVEXML_MODE)
    {
      ret = cargar_XML_biblioteca (FILES_MODE, &image_index2,&audios_index2,&videos_index2,&settings2);
    }
  //errores?
  /* if(MODE)
    recoverDoc("config.xml");
  else
  populateDoc("config.xml");*/
  *image_index = image_index2;  
  *audio_index = audios_index2;
  *video_index = videos_index2;
  *settings = settings2;
  printf("INFO: fin carga_biblioteca!\n");
  return ret;
}
