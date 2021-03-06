#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "ClassRobo.h"

#define REAL 0

int PioneerRobot::isConnected()
{
  return(robot.isConnected());
}


PioneerRobot::PioneerRobot(int tipoConexao,char* info,int *sucesso)
{
  int argc=0;char** argv;
  Aria::init();

  switch (tipoConexao)
	{
    case ConexaoSerial:
		{
      char port1[4];
		  strcpy(port1,info);

			int ret;
			std::string str;

      *sucesso=((ret = con1.open(port1))== 0);
      if (*sucesso)
			{
        robot.setDeviceConnection(&con1);}
        *sucesso=robot.blockingConnect();
	      break;
		 }
	   case ConexaoRadio:
     {
       argc=4;
		   argv =(char **) new char[4];
		   argv[0]=new char[4];
		   argv[1]=new char[20];
       argv[2]=new char[20];
		   argv[3]=new char[7];
		   strcpy(argv[0],"-rh");
		   strcpy(argv[1],info);
		   strcpy(argv[2],"-remoteLaserTcpPort");
		   strcpy(argv[3],"10002");

		   parser=new ArArgumentParser(&argc, argv);
		   robotConnector=new ArRobotConnector(parser,&robot);
       *sucesso=robotConnector->connectRobot();
		   if (!(*sucesso))
		   {
         Aria::shutdown();break;}
         robot.addRangeDevice(&sick);

		     laserConnector=new ArLaserConnector(parser, &robot, robotConnector);
		     laserConnector->setupLaser(&sick);

         break;
		 }

     case ConexaoSimulacao:
		 {
       argc=2;
		   argv=(char **) new char[2];
		   argv[0]=new char[4];
		   argv[1]=new char[20];
		   strcpy(argv[0],"-rh");
		   strcpy(argv[1],"localhost");

		   parser=new ArArgumentParser(&argc, argv);

		   robotConnector=new ArRobotConnector(parser,&robot);
       *sucesso=robotConnector->connectRobot();
		   if (!(*sucesso))
		   {
         Aria::shutdown();
			   break;
       }
		 }
	 } // switch(tipoConexao)

   robot.addRangeDevice(&sick);

   //laserConnector=new ArLaserConnector(parser, &robot, robotConnector);
   //laserConnector->setupLaser(&sick);

   // Used to perform actions when keyboard keys are pressed
   //ArKeyHandler keyHandler;
   //Aria::setKeyHandler(&keyHandler);
   //robot.attachKeyHandler(&keyHandler);

   if(*sucesso)
	 {
     robot.addRangeDevice(&sonarDev);
		 if (*sucesso)
		 {
	     sick.runAsync();
		   robot.setHeading(0);
			 robot.runAsync(true);
			 robot.enableMotors();
			 robot.setRotVelMax(200);
       /*printf("Connecting...\n");
			 if(!laserConnector->connectLaser(&sick))
			 {
         printf("Could not connect to lasers... exiting\n");
         Aria::exit(2);
       }*/

       // Sleep for a second so some messages from the initial responses
       // from robots and cameras and such can catch up
       ArUtil::sleep(1000);

       // We need to lock the robot since we'll be setting up these modes
       // while the robot task loop thread is already running, and they
       // need to access some shared data in ArRobot.
       robot.lock();

       ArModeWander wander(&robot, "wander", 'w', 'W');
       wander.activate();

       // turn on the motors
       robot.comInt(ArCommands::ENABLE, 1);

       robot.unlock();

       // Block execution of the main thread here and wait for the robot's task loop
       // thread to exit (e.g. by robot disconnecting, escape key pressed, or OS
       // signal)
       robot.waitForRunExit();
		 }
		 else Aria::shutdown();
	 }
   else Aria::shutdown();
  }

  void PioneerRobot::destroy() { desconectar(); Aria::shutdown(); }
  void PioneerRobot::pararMovimento() { robot.stop(); }
  void PioneerRobot::desconectar()    { robot.stopRunning(true); }

  int PioneerRobot::getSonar(int i){ return(Sensores[i]);}
  void PioneerRobot::getAllSonar(int *sensores)
  { for(int i=0;i<8;i++) sensores[i]=(int)(robot.getSonarRange(i));
  }
  void PioneerRobot::readSensores()
  { for(int i=0;i<8;i++) Sensores[i]=(int)(robot.getSonarRange(i));
   }
  float PioneerRobot::getXPos()     { return   (robot.getX()/10);}
  float PioneerRobot::getYPos()     { return   (robot.getY()/10);}
  float PioneerRobot::getAngBase () { return   (robot.getTh()); }

  void PioneerRobot::initMov()      {  robot.setVel2(50,50);  }

  void PioneerRobot::getLaser()
  { sick.getRawReadings();
	  std::vector < ArSensorReading > *readings;
    std::vector < ArSensorReading > ::iterator it;

	  FILE* dataFile;
    dataFile = fopen("Laser_log.txt", "w");
    sick.lockDevice();

    readings = sick.getRawReadingsAsVector();
	  if(sick.isConnected())
      fprintf (dataFile, "sim");//,readings->size());
	  else fprintf (dataFile, "nao");//,readings->size());
	  fprintf (dataFile, "%d 2 \n",readings->size());
    for (it = readings->begin(); it!=readings->end(); it++){
      fprintf( dataFile, " %f %f\n" , (*it).getX(), (*it).getY());//,(*it).get );
    }

    sick.unlockDevice();
    fclose ( dataFile );
    ArUtil::sleep(1000);
  }

  void PioneerRobot::Rotaciona(double degrees,int Sentido)
    { robot.setDeltaHeading(degrees);
//	 while (!(robot.isHeadingDone(2))) {};
	 if (Sentido==0)
	     robot.setVel(0);
	 else if (Sentido==1)
	     robot.setVel(50);
	 else if (Sentido==2)
	     robot.setVel(-50);
   }
  void PioneerRobot::Move(double vl,double vr)
  { robot.setVel2(vl,vr);}
