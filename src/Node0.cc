//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Node0.h"

Define_Module(Node0);
void PrintMsgTransmitSender (ofstream & outputFile, double time, int currentSeqNum, string payload, string trailer, int modified, bool lost, int duplicate, string delay)
{
    string islost="";
    if(lost)
        islost="Yes";
    else
        islost="No";
    if(lost)
    {
        outputFile << "At time ["<< time <<"], "
                 "Node[0] [sent] frame with seq_num=["<< currentSeqNum <<"] "
                 "and pay-load=["<<payload<<"] ,"
                 "Lost ["<<islost<<"], "<<"\n";
    }
    else
    {
    outputFile << "At time ["<< time <<"], "
            "Node[0] [sent] frame with seq_num=["<< currentSeqNum <<"] "
            "and pay-load=["<<payload<<"] "
            "and trailer=[ "<<trailer<<" ] , "
            "Modified ["<<modified<<"] , "
            "Lost ["<<islost<<"], "
            "Duplicate ["<<duplicate<<"], "
            "Delay ["<<delay<<"] "<<"\n";
    }
}
MyMessage_Base * Create(int currentSeqNumber, vector<string>DataVector)
{
    MyMessage_Base * msg = new MyMessage_Base("");

    string message=DataVector[currentSeqNumber];

    int messageSize=message.size();

    msg->setFrame_Payload(message.c_str());
    msg->setFrame_Type(DATA_TYPE);//data
    msg->setSeq_Num(currentSeqNumber);//awel msg

    //calculate checksum byte
    bitset<8> checkSumByte(0);
    for(int i=0;i<messageSize;i++)
    {
        bitset<8> xbits(message[i]);
        checkSumByte=checkSumByte^xbits;//parity bit
    }
    msg->setChecksum(checkSumByte);

    EV<<"NODE 0 sent message with sequence number ...   ";
    EV << msg->getSeq_Num();
    EV<<"  and payload of ... ";
    EV<< msg->getFrame_Payload();
    EV<<"   and check bits of ...";
    EV<< msg->getChecksum().to_string();
    EV<<endl;
    return msg;

}

void ByteStuffing(MyMessage_Base * msg,vector<int> &TimeOut)
{
    string newPayloadAfterByteStuffing="#";
    string payload=msg->getFrame_Payload();
    int payloadSize=payload.length();
    for(int i=0;i<payloadSize;i++)
    {
        if(payload[i]=='#' || payload[i]=='/')
            newPayloadAfterByteStuffing+='/';
        newPayloadAfterByteStuffing+=payload[i];
    }
    newPayloadAfterByteStuffing+='#';
    msg->setFrame_Payload(newPayloadAfterByteStuffing.c_str());
}
void Node0::initialize()
{
    // TODO - Generated method body
        //int receiver
        for(int i=0;i<int(getParentModule()->par("WR")) ;i++)
        {
            recievedFrames.push_back("");
        }
       //init sender
        string myText;
        ifstream MyFile("input0.txt");
        if (!MyFile.is_open()) {
            cout << "Error: Unable to open the file 'coordinator.txt'" << endl;
            return;
        }
        while(getline (MyFile, myText))
        {
            string errortype=myText.substr(0,4);
            string data=myText.substr(5,myText.size());
            TypeOfErrorsVector.push_back(errortype);
            DataVector.push_back(data);
        }

        MyFile.close();


}

void Node0::handleMessage(cMessage *msg)
{
  // TODO - Generated method body
   outputFile.open("output.txt", ios_base::app);
   if(strcmp(msg->getName(),"start")==0)//msg from coordinator->I am sender
   {
       double delay=double(getParentModule()->par("TD"));
       simtime_t ptD=simTime();
       double ptDelay=ptD.dbl()+double(getParentModule()->par("PT"));

       for(int i=0;i<3;i++)
       {
           if(currentSeqNumber>=DataVector.size())
               break;
           MyMessage_Base * msg=Create(currentSeqNumber, DataVector);
           string typeOfError=TypeOfErrorsVector[currentSeqNumber];
           //logfile
           outputFile << "At time [" << (simTime()+(i*double(getParentModule()->par("PT")))) << "], "
                           "Node[0] , Introducing channel error with "
                           "code =[" << typeOfError <<"] "<<"\n";

           if(typeOfError[1]=='1')//loss -> x1xx
           {
               delay+=double(getParentModule()->par("PT"));
               string payload=(msg->getFrame_Payload());
               PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, payload, msg->getChecksum().to_string(),-1, true ,0,to_string(0));

               double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));

               TimeOut.push_back(msg->getSeq_Num());
               scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
           }
           else if(typeOfError=="0001")//delay
           {
               double tempDelay=delay+double(getParentModule()->par("ED"))+double(getParentModule()->par("PT"));
               //timer
               double endTimer=tempDelay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
               scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));

               delay+=double(getParentModule()->par("PT"));
               //logfile
               string payload=(msg->getFrame_Payload());
               PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, payload, msg->getChecksum().to_string(),-1, false,0,to_string(ptDelay)+" - "+to_string(ptDelay+double(getParentModule()->par("ED"))) );

               //Byte stuffing
               ByteStuffing(msg,TimeOut);
               TimeOut.push_back(msg->getSeq_Num());
               sendDelayed(msg,tempDelay,"out");

           }
           else if(typeOfError=="0010")//duplicate
           {
               string payload=(msg->getFrame_Payload());

               //original
               delay+=double(getParentModule()->par("PT"));
               double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
               PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, payload, msg->getChecksum().to_string(),-1, false,1,to_string(0));
               PrintMsgTransmitSender(outputFile, ptDelay+double(getParentModule()->par("DD")), currentSeqNumber, payload, msg->getChecksum().to_string(),-1, false,2,to_string(0));

               scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
               //Byte stuffing
               ByteStuffing(msg,TimeOut);
               TimeOut.push_back(msg->getSeq_Num());
               sendDelayed(msg,delay,"out");
               //dup
               double tempDelay=delay+double(getParentModule()->par("DD"));
               sendDelayed(msg->dup(),tempDelay,"out");
           }
           else if(typeOfError=="1000")//modification
           {
               string msgPayload=msg->getFrame_Payload();
               int errorBit=int(uniform(0,((msgPayload.length())*8)-1));//error in any bit including char count and parity
               int errorBitPrint=errorBit;

               int errorChar=errorBit/8;
               errorBit=(errorBit-1)%8;

               bitset<8> erroredCharBits(msgPayload[errorChar]);
               erroredCharBits[errorBit]=!erroredCharBits[errorBit];
               msgPayload[errorChar]=(char)(erroredCharBits).to_ulong() ;
               msg->setFrame_Payload(msgPayload.c_str());

               PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, msgPayload, msg->getChecksum().to_string(),errorBitPrint, false,0,to_string(0));

               delay+=double(getParentModule()->par("PT"));
               double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
               scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
               //Byte stuffing
               ByteStuffing(msg,TimeOut);
               TimeOut.push_back(msg->getSeq_Num());
               sendDelayed(msg,delay,"out");
           }
           else if(typeOfError=="0000")//no error
           {
               delay+=double(getParentModule()->par("PT"));
               string payload=(msg->getFrame_Payload());
               PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, payload, msg->getChecksum().to_string(),-1, false,0,to_string(0));
               double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
               //Byte stuffing
               ByteStuffing(msg,TimeOut);
               TimeOut.push_back(msg->getSeq_Num());
               //send
               if(i==0)
                   test=msg;
               scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
               sendDelayed(msg,delay,"out");
           }
           else if(typeOfError=="0011")//Delay+Duplication
           {
               //delay
               double tempDelay=delay+double(getParentModule()->par("ED"))+double(getParentModule()->par("PT"));
               double endTimer=tempDelay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
               scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
               //logfile
               string payload=(msg->getFrame_Payload());
               PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, payload, msg->getChecksum().to_string(),-1, false,1,to_string(ptDelay)+" - "+to_string(ptDelay+double(getParentModule()->par("ED"))) );
               PrintMsgTransmitSender(outputFile, ptDelay+double(getParentModule()->par("DD")), currentSeqNumber, payload, msg->getChecksum().to_string(),-1, false,2,to_string(ptDelay)+" - "+to_string(ptDelay+double(getParentModule()->par("ED"))) );
               //Byte stuffing
               ByteStuffing(msg,TimeOut);
               TimeOut.push_back(msg->getSeq_Num());
               sendDelayed(msg,tempDelay,"out");
               //dup
               tempDelay+=double(getParentModule()->par("DD"));
               sendDelayed(msg->dup(),tempDelay,"out");
               //lel b3dya
               delay+=double(getParentModule()->par("PT"));


           }
           else if(typeOfError=="1010")//Modification+Duplication
           {
               //modify
               string msgPayload=msg->getFrame_Payload();
               int errorBit=int(uniform(0,((msgPayload.length())*8)-1));//error in any bit including char count and parity
               int errorBitPrint=errorBit;

               int errorChar=errorBit/8;
               errorBit=(errorBit-1)%8;

               bitset<8> erroredCharBits(msgPayload[errorChar]);
               erroredCharBits[errorBit]=!erroredCharBits[errorBit];
               msgPayload[errorChar]=(char)(erroredCharBits).to_ulong() ;
               msg->setFrame_Payload(msgPayload.c_str());

               //send first modify
               delay+=double(getParentModule()->par("PT"));
               double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
               scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
               //log file
               PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, msgPayload, msg->getChecksum().to_string(),errorBitPrint, false,1,to_string(0));
               PrintMsgTransmitSender(outputFile, ptDelay+double(getParentModule()->par("DD")), currentSeqNumber, msgPayload, msg->getChecksum().to_string(),errorBitPrint, false,2,to_string(0));

               //Byte stuffing
               ByteStuffing(msg,TimeOut);
               TimeOut.push_back(msg->getSeq_Num());
               sendDelayed(msg,delay,"out");

               //dup
               double tempDelay=delay+double(getParentModule()->par("DD"));
               sendDelayed(msg->dup(),tempDelay,"out");
           }
           else if(typeOfError=="1011")//Modification+Duplication+Delay
           {
               //modify
               string msgPayload=msg->getFrame_Payload();
               int errorBit=int(uniform(0,((msgPayload.length())*8)-1));//error in any bit including char count and parity
               int errorBitPrint=errorBit;

               int errorChar=errorBit/8;
               errorBit=(errorBit-1)%8;

               bitset<8> erroredCharBits(msgPayload[errorChar]);
               erroredCharBits[errorBit]=!erroredCharBits[errorBit];
               msgPayload[errorChar]=(char)(erroredCharBits).to_ulong() ;
               msg->setFrame_Payload(msgPayload.c_str());

               //delay el 1st modified
               double tempDelay=delay+double(getParentModule()->par("ED"))+double(getParentModule()->par("PT"));
               double endTimer=tempDelay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
               scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
               //logfile
               string payload=(msg->getFrame_Payload());
               PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, payload, msg->getChecksum().to_string(),errorBitPrint, false,1,to_string(ptDelay)+" - "+to_string(ptDelay+double(getParentModule()->par("ED"))) );
               PrintMsgTransmitSender(outputFile, ptDelay+double(getParentModule()->par("DD")), currentSeqNumber, payload, msg->getChecksum().to_string(),errorBitPrint, false,2,to_string(ptDelay)+" - "+to_string(ptDelay+double(getParentModule()->par("ED"))) );

               //Byte stuffing
               ByteStuffing(msg,TimeOut);
               TimeOut.push_back(msg->getSeq_Num());
               sendDelayed(msg,tempDelay,"out");

               //dup
               tempDelay+=double(getParentModule()->par("DD"));
               sendDelayed(msg->dup(),tempDelay,"out");
               //lel b3dya
               delay+=double(getParentModule()->par("PT"));

           }
           currentSeqNumber++;
           ptDelay+=double(getParentModule()->par("PT"));
       }

   }
   else
   {
       if(msg->isSelfMessage())//timeout
       {
          int seqnumber=atoi(msg->getName());
          if(find(TimeOut.begin(),TimeOut.end(),seqnumber)!=TimeOut.end())
          {
             cout<<"TIMEOUT"<<seqnumber<<endl;
             MyMessage_Base * msg=Create(seqnumber, DataVector);
             outputFile << "Time out event at time [" << simTime() << "], "
                                        "Node[0] , for frame with seq_num"
                                        " =[" << seqnumber <<"] "<<"\n";
             double delay=double(getParentModule()->par("TD"))+double(getParentModule()->par("PT"));
             double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
             scheduleAt(simTime()+endTimer,new cMessage((to_string(seqnumber)).c_str()));
             TimeOut.erase(remove(TimeOut.begin(), TimeOut.end(),seqnumber));
             //logtime
            simtime_t ptD=simTime();
            double ptdelay=ptD.dbl()+double(getParentModule()->par("PT"));
            string payload=(msg->getFrame_Payload());
            PrintMsgTransmitSender(outputFile, ptdelay, seqnumber, payload, msg->getChecksum().to_string(),-1, false,0,"0" );

             ByteStuffing(msg,TimeOut);
             sendDelayed(msg,delay,"out");
          }
       }
       else
       {
           MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);
           if(mmsg->getFrame_Type()==ACK_TYPE)
           {
               EV<<"NODE 0 recieved ack with sequence number ...   ";
               EV << mmsg->getSeq_Num();
               EV<<endl;
               outputFile << "At time [" << simTime() << "], "
                             "Node[0] recieved Ack with"
                             "seq_num =[" << mmsg->getSeq_Num() <<"]\n";
               ackCounter++;
               if(ackCounter==DataVector.size())
               {
                   outputFile.close();
                   endSimulation();
               }
               cout<<"TO SIZEE "<<TimeOut.size()<<endl;
                  for(int i=0;i<TimeOut.size();i++)
                 {
                     cout<<"TO BEFORE "<<TimeOut[i]<<endl;
                 }
              if(find(TimeOut.begin(),TimeOut.end(),mmsg->getSeq_Num()-1)!=TimeOut.end())
              {
                  cout<<"REMOVEEE"<<mmsg->getSeq_Num()-1<<endl;
                  TimeOut.erase(remove(TimeOut.begin(), TimeOut.end(), mmsg->getSeq_Num()-1));
                  for(int i=0;i<TimeOut.size();i++)
                  {
                      cout<<"TO AFTER "<<TimeOut[i]<<endl;
                  }
              }
              if(expectedSeqNumber==mmsg->getSeq_Num()) // Ack recieved is equal to ack expected
              {
                  int slidepointer=1; //slide window by one
                  expectedSeqNumber++; // increment expected seq number
                  while(find(Acks.begin(),Acks.end(),expectedSeqNumber)!=Acks.end())//if the next ack was recieved before
                  {
                      Acks.erase(remove(Acks.begin(), Acks.end(), expectedSeqNumber)); //remove it from array of acks
                       expectedSeqNumber++;
                       slidepointer++; //slide window again
                  }
                  double delay=double(getParentModule()->par("TD"));
                  simtime_t ptD=simTime();
                  double ptDelay=ptD.dbl()+double(getParentModule()->par("PT"));
                  for(int i=0;i<slidepointer;i++) //send new messages after sliding the window
                  {
                     if(currentSeqNumber>=DataVector.size())
                         break;
                     MyMessage_Base * msg=Create(currentSeqNumber, DataVector);
                     string typeOfError=TypeOfErrorsVector[currentSeqNumber];
                     //logfile
                     outputFile << "At time [" << (simTime()+(i*double(getParentModule()->par("PT")))) << "], "
                                 "Node[0] , Introducing channel error with "
                                 "code =[" << typeOfError <<"] "<<"\n";
                     if(typeOfError[1]=='1')//loss -> x1xx
                     {
                         delay+=double(getParentModule()->par("PT"));
                         string payload=(msg->getFrame_Payload());
                         PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, payload, msg->getChecksum().to_string(),-1, true ,0,to_string(0));
                         TimeOut.push_back(msg->getSeq_Num());

                         double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                         scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                     }
                     else if(typeOfError=="0001")//delay
                     {
                         double tempDelay=delay+double(getParentModule()->par("ED"))+double(getParentModule()->par("PT"));
                         double endTimer=tempDelay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                         scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                         //logfile
                         string payload=(msg->getFrame_Payload());
                         PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, payload, msg->getChecksum().to_string(),-1, false,0,to_string(ptDelay)+" - "+to_string(ptDelay+double(getParentModule()->par("ED"))) );

                         //Byte stuffing
                         ByteStuffing(msg,TimeOut);
                         TimeOut.push_back(msg->getSeq_Num());
                         sendDelayed(msg,tempDelay,"out");
                         delay+=double(getParentModule()->par("PT"));
                     }
                     else if(typeOfError=="0010")//duplicate
                     {
                         //original
                         delay+=double(getParentModule()->par("PT"));
                         double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                         //logfile
                         string payload=(msg->getFrame_Payload());
                         PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, payload, msg->getChecksum().to_string(),-1, false,1,to_string(0));
                         PrintMsgTransmitSender(outputFile, ptDelay+double(getParentModule()->par("DD")), currentSeqNumber, payload, msg->getChecksum().to_string(),-1, false,2,to_string(0));

                         scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));

                         //Byte stuffing
                         ByteStuffing(msg,TimeOut);
                         TimeOut.push_back(msg->getSeq_Num());
                         sendDelayed(msg,delay,"out");
                         //dup
                         double tempDelay=delay+double(getParentModule()->par("DD"));
                         sendDelayed(msg->dup(),tempDelay,"out");
                     }
                     else if(typeOfError=="1000")//modification
                     {
                         string msgPayload=msg->getFrame_Payload();
                         int errorBit=int(uniform(0,((msgPayload.length())*8)-1));//error in any bit including char count and parity
                         int errorBitPrint=errorBit;

                         int errorChar=errorBit/8;
                         errorBit=(errorBit-1)%8;

                         bitset<8> erroredCharBits(msgPayload[errorChar]);
                         erroredCharBits[errorBit]=!erroredCharBits[errorBit];
                         msgPayload[errorChar]=(char)(erroredCharBits).to_ulong() ;
                         msg->setFrame_Payload(msgPayload.c_str());

                         //logfile
                         PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, msgPayload, msg->getChecksum().to_string(),errorBitPrint, false,0,to_string(0));

                         delay+=double(getParentModule()->par("PT"));
                         double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                         scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                         //Byte stuffing
                         ByteStuffing(msg,TimeOut);
                         TimeOut.push_back(msg->getSeq_Num());
                         sendDelayed(msg,delay,"out");
                     }
                     else if(typeOfError=="0000")//no error
                     {
                         delay+=double(getParentModule()->par("PT"));
                         double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                         //logfile
                         string payload=(msg->getFrame_Payload());
                         PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, payload, msg->getChecksum().to_string(),-1, false,0,to_string(0));

                         //Byte stuffing
                         ByteStuffing(msg,TimeOut);
                         TimeOut.push_back(msg->getSeq_Num());
                         //send
                         scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                         sendDelayed(msg,delay,"out");
                     }
                     else if(typeOfError=="0011")//Delay+Duplication
                     {
                         //delay
                         double tempDelay=delay+double(getParentModule()->par("ED"))+double(getParentModule()->par("PT"));
                         double endTimer=tempDelay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                         scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                         //logfile
                         string payload=(msg->getFrame_Payload());
                         PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, payload, msg->getChecksum().to_string(),-1, false,1,to_string(ptDelay)+" - "+to_string(ptDelay+double(getParentModule()->par("ED"))) );
                         PrintMsgTransmitSender(outputFile, ptDelay+double(getParentModule()->par("DD")), currentSeqNumber, payload, msg->getChecksum().to_string(),-1, false,2,to_string(ptDelay)+" - "+to_string(ptDelay+double(getParentModule()->par("ED"))) );

                         //Byte stuffing
                         ByteStuffing(msg,TimeOut);
                         TimeOut.push_back(msg->getSeq_Num());
                         sendDelayed(msg,tempDelay,"out");
                         //dup
                         tempDelay+=double(getParentModule()->par("DD"));
                         sendDelayed(msg->dup(),tempDelay,"out");
                         //lel b3dya
                         delay+=double(getParentModule()->par("PT"));
                     }
                     else if(typeOfError=="1010")//Modification+Duplication
                     {
                         //modify
                         string msgPayload=msg->getFrame_Payload();
                         int errorBit=int(uniform(0,((msgPayload.length())*8)-1));//error in any bit including char count and parity
                         int errorBitPrint=errorBit;

                         int errorChar=errorBit/8;
                         errorBit=(errorBit-1)%8;

                         bitset<8> erroredCharBits(msgPayload[errorChar]);
                         erroredCharBits[errorBit]=!erroredCharBits[errorBit];
                         msgPayload[errorChar]=(char)(erroredCharBits).to_ulong() ;
                         msg->setFrame_Payload(msgPayload.c_str());

                         //send first modify
                         delay+=double(getParentModule()->par("PT"));
                         double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                         scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                         //log file
                         PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, msgPayload, msg->getChecksum().to_string(),errorBitPrint, false,1,to_string(0));
                         PrintMsgTransmitSender(outputFile, ptDelay+double(getParentModule()->par("DD")), currentSeqNumber, msgPayload, msg->getChecksum().to_string(),errorBitPrint, false,2,to_string(0));

                         //Byte stuffing
                         ByteStuffing(msg,TimeOut);
                         TimeOut.push_back(msg->getSeq_Num());
                         sendDelayed(msg,delay,"out");

                         //dup
                         double tempDelay=delay+double(getParentModule()->par("DD"));
                         sendDelayed(msg->dup(),tempDelay,"out");
                     }
                     else if(typeOfError=="1011")//Modification+Duplication+Delay
                     {
                         //modify
                         string msgPayload=msg->getFrame_Payload();
                         int errorBit=int(uniform(0,((msgPayload.length())*8)-1));//error in any bit including char count and parity
                         int errorBitPrint=errorBit;

                         int errorChar=errorBit/8;
                         errorBit=(errorBit-1)%8;

                         bitset<8> erroredCharBits(msgPayload[errorChar]);
                         erroredCharBits[errorBit]=!erroredCharBits[errorBit];
                         msgPayload[errorChar]=(char)(erroredCharBits).to_ulong() ;
                         msg->setFrame_Payload(msgPayload.c_str());

                         //delay el 1st modified
                         double tempDelay=delay+double(getParentModule()->par("ED"))+double(getParentModule()->par("PT"));
                         double endTimer=tempDelay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                         scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                         //logfile
                         string payload=(msg->getFrame_Payload());
                         PrintMsgTransmitSender(outputFile, ptDelay, currentSeqNumber, payload, msg->getChecksum().to_string(),errorBitPrint, false,1,to_string(ptDelay)+" - "+to_string(ptDelay+double(getParentModule()->par("ED"))) );
                         PrintMsgTransmitSender(outputFile, ptDelay+double(getParentModule()->par("DD")), currentSeqNumber, payload, msg->getChecksum().to_string(),errorBitPrint, false,2,to_string(ptDelay)+" - "+to_string(ptDelay+double(getParentModule()->par("ED"))) );

                         //Byte stuffing
                         ByteStuffing(msg,TimeOut);
                         TimeOut.push_back(msg->getSeq_Num());
                         sendDelayed(msg,tempDelay,"out");

                         //dup
                         tempDelay+=double(getParentModule()->par("DD"));
                         sendDelayed(msg->dup(),tempDelay,"out");
                         //lel b3dya
                         delay+=double(getParentModule()->par("PT"));
                     }
                     currentSeqNumber++;
                     ptDelay+=double(getParentModule()->par("PT"));

                  }
              }
              else
              {
                  Acks.push_back(mmsg->getSeq_Num());
              }
           }
           else if(mmsg->getFrame_Type()==NACK_TYPE)
           {
               EV<<"NODE 0 recieved NACK with sequence number ...   ";
               EV << mmsg->getSeq_Num();
               EV<<endl;
               outputFile << "At time [" << simTime() << "], "
                             "Node[0] recived Nack with"
                             "seq_num =[" << mmsg->getSeq_Num() <<"]\n";
              int seqnumber=mmsg->getSeq_Num();
              MyMessage_Base * msg=Create(seqnumber, DataVector);
              double delay=double(getParentModule()->par("TD"))+double(getParentModule()->par("PT"));
              double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
              scheduleAt(simTime()+endTimer,new cMessage((to_string(seqnumber)).c_str()));

              //logtime
               simtime_t ptD=simTime();
               double ptdelay=ptD.dbl()+double(getParentModule()->par("PT"));
               string payload=(msg->getFrame_Payload());
               PrintMsgTransmitSender(outputFile, ptdelay, seqnumber, payload, msg->getChecksum().to_string(),-1, false,0,"0" );

              ByteStuffing(msg,TimeOut);
              sendDelayed(msg,delay,"out");
           }
           else
           {
           //Reciever CODE
           MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);
           if(mmsg->getFrame_Type()==DATA_TYPE)
           {
               //recived galy msg ha check errord b checksum(b3d ma asheel el control bytes) law errored->nack law la ack
               //law ack: law heya el expected -> shift window cout el amltelha shift
               //w b3den abos al vector law el b3dya mawgoda ha cout w a set el mkann el fl vector b empty string aw ayan kan
               //w abos al b3dha ->>> while loop

               //get old payload
               string payload=mmsg->getFrame_Payload();

               //Byte unstuffing
               string newPayload="";
               int payloadSize=payload.length();
               for(int i=1;i<payloadSize-1;i++)
               {
                   if((payload[i]=='/' ))
                   {
                       newPayload+=payload[i+1];
                       i++;
                   }
                   else
                       newPayload+=payload[i];
               }
               EV<<"NODE 1 recieved message with sequence number ...   ";
               EV << mmsg->getSeq_Num();
               EV<<"  and payload of ... ";
               EV<< mmsg->getFrame_Payload();
               EV<<"   and check bits of ...";
               EV<< mmsg->getChecksum().to_string();
               EV<<"   and at time ...";
               EV<< simTime();
               EV<<endl;
               payloadSize=newPayload.length();
               //calculate checksum
               bitset<8> checkSumByte(0);
               for(int i=0;i<payloadSize;i++)
               {
                   bitset<8> xbits(newPayload[i]);
                   checkSumByte=checkSumByte^xbits;//parity bit
               }

               outputFile << "At time [" << simTime() << "], "
                             "Node[0] received frame with"
                             "seq_num =[" << mmsg->getSeq_Num() <<"] "
                             "with payload =[" << payload <<"] "
                             "and trailer=[ "<<checkSumByte<<" ] , "
                             "Modified [-1] , "
                             "Lost [No], "
                             "Duplicate [0], "
                             "Delay [0] "<<"\n";


               if(checkSumByte == mmsg->getChecksum())
               {
                   int recievedSeqNum=mmsg->getSeq_Num();
                   if(recievedSeqNum<expectedSeqNumberRec)
                       return;
                   else if(recievedSeqNum==expectedSeqNumberRec)
                   {
                       int slidepointer=1; //slide window by one
                       expectedSeqNumberRec++;
                       while(recievedFrames[expectedSeqNumberRec%int(getParentModule()->par("WR"))]!="")
                      {
                           recievedFrames[expectedSeqNumberRec%int(getParentModule()->par("WR"))]="";
                           expectedSeqNumberRec++;
                           slidepointer++; //slide window again
                      }
                   }
                   else
                   {
                       if(recievedFrames[recievedSeqNum%int(getParentModule()->par("WR"))]!="")
                           return;
                       recievedFrames[recievedSeqNum%int(getParentModule()->par("WR"))]=newPayload;
                   }
                   mmsg->setFrame_Type(ACK_TYPE);//ACK
                   mmsg->setSeq_Num(mmsg->getSeq_Num()+1);
                   outputFile << "At time [" << simTime() +double(getParentModule()->par("PT"))<< "], "
                                 "Node[0] sending Ack with"
                                 "seq_num =[" << mmsg->getSeq_Num() <<"]\n";

               }
               else
               {
                   mmsg->setFrame_Type(NACK_TYPE);//NACK
                   mmsg->setSeq_Num(mmsg->getSeq_Num());
                   outputFile << "At time [" << simTime() +double(getParentModule()->par("PT"))<< "], "
                                 "Node[0] sending Nack with"
                                 "seq_num =[" << mmsg->getSeq_Num() <<"]\n";
               }
               mmsg->setFrame_Payload("");
               sendDelayed(mmsg,double(getParentModule()->par("TD"))+double(getParentModule()->par("PT")),"out");
           }
          }
       }
   }
   outputFile.close();
}
