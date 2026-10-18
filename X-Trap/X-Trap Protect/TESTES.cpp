
#include "StdAfx.h"


void main()
{
  while (true){
	  Sleep (5000);
	  cout<< "TESTE.cpp: " <<  phathien.Ten_Cua_So <<endl;
}
}



void  TESTES(){	
 CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(main),NULL,0,0);	
}
 


