#include"define.h"
#include<iostream>
#include<fstream>
#include<sstream>
#include<random>
#include<time.h>
#include<algorithm>

#define outputPAT 0									//1: output PAT to txt file ; 0: store in program memory
#define outputUEinfo 0								//1: output UE information to txt file ; 0: store in program memory

using namespace std;

void OutputResult(string Scheme, SimulationResult *Result);

int LTECQIRange[] = { 1732, 1511, 1325, 1162, 1019, 894, 784, 688, 623, 565, 512, 449, 407, 357, 303 };
UE UEList[UEnumber];
int DB50_UEnumber = 0;
int DB100_UEnumber = 0;
int DB300_UEnumber = 0;
vector <double> TempPacketArrivalTime[UEnumber];		//�ΨӼȦsUE��packet pattern

double CQIEfficiency(int CQI)
{
	double efficiency = 0.0;
	switch (CQI)
	{
	case 1:
		efficiency = 0.1523;
		break;
	case 2:
		efficiency = 0.2344;
		break;
	case 3:
		efficiency = 0.3770;
		break;
	case 4:
		efficiency = 0.6016;
		break;
	case 5:
		efficiency = 0.8770;
		break;
	case 6:
		efficiency = 1.1758;
		break;
	case 7:
		efficiency = 1.4766;
		break;
	case 8:
		efficiency = 1.9141;
		break;
	case 9:
		efficiency = 2.4063;
		break;
	case 10:
		efficiency = 2.7305;
		break;
	case 11:
		efficiency = 3.3223;
		break;
	case 12:
		efficiency = 3.9023;
		break;
	case 13:
		efficiency = 4.5234;
		break;
	case 14:
		efficiency = 5.1152;
		break;
	case 15:
		efficiency = 5.5547;
		break;
	default:
		break;
	}
	return efficiency;
}

template <class T>
void uniformdistribution(T* equip)
{
	std::random_device rd;							//integer random number generator that produces non-deterministic random numbers. 
	std::mt19937 gen(rd());							//Mersenne Twister 19937 generator, generate a random number seed
	std::uniform_real_distribution<> theta(0, 360);	//definition of a uniform distribution range, a random number between 0 and 360
	std::uniform_real_distribution<> k(0, 1);		//definition of a uniform distribution range, a random number between 0 and 1
	double r = radius_eNB * sqrt(k(gen));			//random a angle and random a radius, to gennerate a coordinate for UE
	double angel = (theta(gen));
	equip->coor_X = r * std::sin(angel);
	equip->coor_Y = r * std::cos(angel);
}

double exponentially_Distributed(double x)
{
	double y, z;
	y = (double)(rand() + 1) / (double)(RAND_MAX + 1);
	z = (double)log(y) * (double)(-1 / x);
	return z;
}

double getDistance(double coor_x, double coor_y) { return sqrt(pow(coor_x, 2) + pow(coor_y, 2)); }

int getCQI(UE *u)
{
	double distance = getDistance(u->coor_X, u->coor_Y);
	int CQI = 0;
	for (int i = 0; i < 15; i++)
	{
		if (distance <= LTECQIRange[i])
			CQI++;
		else
			break;
	}
	return CQI;
}

// int��string
string IntToString(int &i)
{
	string s;
	stringstream ss(s);
	ss << i;
	return ss.str();
}

void Simulation_Result(UE *UEList, SimulationResult *Result)
{
	//Queueing model calculation
	double AvgSystemTime = 0.0;
	double Xj = 0.0;
	double Xj2 = 0.0;
	double lambda = 0.0;
	for (int i = 0; i < UEnumber; i++)
		lambda = lambda + UEList[i].lambdai;
	for (int i = 0; i < UEnumber; i++)
	{
		double weight_i = UEList[i].lambdai / lambda;
		double Xij = UEList[i].packet_size / (resource_element * CQIEfficiency(UEList[i].CQI) * total_RBG);
		Xj = Xj + Xij * weight_i;
		Xj2 = Xj2 + pow(Xij, 2) * weight_i;
	}
	Result->Rho = lambda * Xj;
	Result->AvgSystemTime = Xj + lambda * Xj2 / (1 - lambda * Xj);

	// �p����骺throughput�Bdelay�Bschedule packet�ơBdiscard packet��
	double DelayTemp = 0.0;
	double TransmissionTimeTemp = 0.0;
	double SystemTimeTemp = 0.0;
	double Type1_DelayTemp = 0.0;
	double Type2_DelayTemp = 0.0;
	double Type3_DelayTemp = 0.0;
	for (int i = 0; i<UEnumber; i++)
	{
		Result->TotalThroughput += (Result->Throughput[i] / 1000000);			//(Kbps)
		DelayTemp += Result->Delay[i];
		SystemTimeTemp += Result->SystemTime[i];
		TransmissionTimeTemp += Result->TransmissionTime[i];
		Result->TotalSchedulePacketNum = Result->TotalSchedulePacketNum + Result->SchedulePacketNum[i];
		Result->TotalDiscardPacketNum = Result->TotalDiscardPacketNum + Result->DiscardPacketNum[i];
		Result->TotalRemainPacketNum += Result->RemainPacketNum[i];
	}
	Result->AverageThroughput = Result->TotalThroughput / UEnumber;
	Result->AverageDelay = DelayTemp / UEnumber;
	Result->AverageTransmissionTime = TransmissionTimeTemp / Result->TotalSchedulePacketNum;
	Result->AverageSystemTime = SystemTimeTemp / UEnumber;
	Result->PacketLossRatio = ((double)Result->TotalRemainPacketNum / (double)(Result->TotalSchedulePacketNum + Result->TotalRemainPacketNum)) * 100;

	//// �p��typ1(VoIP)��throughput�Bdelay�Bschedule packet�ơBdiscard packet�ơBrate���N�סBdelay���N��
	//if (DB50_UEnumber > 0)
	//{
	//	for (int i = 0; i<DB50_UEnumber; i++)
	//	{
	//		Result->Type1_TotalThroughput = Result->Type1_TotalThroughput + Result->Throughput[i];
	//		Type1_DelayTemp = Type1_DelayTemp + (Result->Delay[i] / Result->SchedulePacketNum[i]);
	//		Result->Type1_SchedulePacketNum = Result->Type1_SchedulePacketNum + Result->SchedulePacketNum[i];
	//		Result->Type1_DiscardPacketNum = Result->Type1_DiscardPacketNum + Result->DiscardPacketNum[i];
	//		Result->RateSatisfaction[i] = (((Result->Throughput[i] * 1000000) / simulation_time) / UEList[i].bit_rate) * 100;
	//		if (Result->RateSatisfaction[i] >= 100)
	//			Result->RateSatisfaction[i] = 100;
	//		Result->DelaySatisfaction[i] = ((double)Result->SchedulePacketNum[i] / (double)(Result->DiscardPacketNum[i] + Result->SchedulePacketNum[i])) * 100;
	//	}
	//	Result->Type1_AverageThroughput = Result->Type1_TotalThroughput / DB50_UEnumber;
	//	Result->Type1_AverageDelay = Type1_DelayTemp / DB50_UEnumber;
	//	Result->Type1_PacketLossRatio = ((double)Result->Type1_DiscardPacketNum / (double)(Result->Type1_SchedulePacketNum + Result->Type1_DiscardPacketNum)) * 100;
	//}

	//// �p��type2(Video)��throughput�Bdelay�Bschedule packet�ơBdiscard packet�ơBrate���N�סBdelay���N��
	//if (DB100_UEnumber > 0)
	//{
	//	for (int i = DB50_UEnumber; i<DB50_UEnumber + DB100_UEnumber; i++)
	//	{
	//		Result->Type2_TotalThroughput = Result->Type2_TotalThroughput + Result->Throughput[i];
	//		Type2_DelayTemp = Type2_DelayTemp + (Result->Delay[i] / Result->SchedulePacketNum[i]);
	//		Result->Type2_SchedulePacketNum = Result->Type2_SchedulePacketNum + Result->SchedulePacketNum[i];
	//		Result->Type2_DiscardPacketNum = Result->Type2_DiscardPacketNum + Result->DiscardPacketNum[i];
	//		Result->RateSatisfaction[i] = (((Result->Throughput[i] * 1000000) / simulation_time) / UEList[i].bit_rate) * 100;
	//		if (Result->RateSatisfaction[i] >= 100)
	//			Result->RateSatisfaction[i] = 100;
	//		Result->DelaySatisfaction[i] = ((double)Result->SchedulePacketNum[i] / (double)(Result->DiscardPacketNum[i] + Result->SchedulePacketNum[i])) * 100;
	//	}
	//	Result->Type2_AverageThroughput = Result->Type2_TotalThroughput / DB100_UEnumber;
	//	Result->Type2_AverageDelay = Type2_DelayTemp / DB100_UEnumber;
	//	Result->Type2_PacketLossRatio = ((double)Result->Type2_DiscardPacketNum / (double)(Result->Type2_SchedulePacketNum + Result->Type2_DiscardPacketNum)) * 100;
	//}

	//// �p��type3��throughput�Bdelay�Bschedule packet�ơBdiscard packet�ơBrate���N�סBdelay���N��
	//if (DB300_UEnumber > 0)
	//{
	//	for (int i = DB50_UEnumber + DB100_UEnumber; i<DB50_UEnumber + DB100_UEnumber + DB300_UEnumber; i++)
	//	{
	//		Result->Type3_TotalThroughput = Result->Type3_TotalThroughput + Result->Throughput[i];
	//		Type3_DelayTemp = Type3_DelayTemp + (Result->Delay[i] / Result->SchedulePacketNum[i]);
	//		Result->Type3_SchedulePacketNum = Result->Type3_SchedulePacketNum + Result->SchedulePacketNum[i];
	//		Result->Type3_DiscardPacketNum = Result->Type3_DiscardPacketNum + Result->DiscardPacketNum[i];
	//		Result->RateSatisfaction[i] = (((Result->Throughput[i] * 1000000) / simulation_time) / UEList[i].bit_rate) * 100;
	//		if (Result->RateSatisfaction[i] >= 100)
	//			Result->RateSatisfaction[i] = 100;
	//		Result->DelaySatisfaction[i] = ((double)Result->SchedulePacketNum[i] / (double)(Result->DiscardPacketNum[i] + Result->SchedulePacketNum[i])) * 100;
	//	}
	//	Result->Type3_AverageThroughput = Result->Type3_TotalThroughput / DB300_UEnumber;
	//	Result->Type3_AverageDelay = Type3_DelayTemp / DB300_UEnumber;
	//	Result->Type3_PacketLossRatio = ((double)Result->Type3_DiscardPacketNum / (double)(Result->Type3_SchedulePacketNum + Result->Type3_DiscardPacketNum)) * 100;
	//}
	OutputResult("EqualRB", Result);
}

void OutputResult(string Scheme, SimulationResult *Result)
{
	fstream Write_SimulationResultFile;              // �ŧifstream����A�ΨӦs��XSystemtime
	string SimulationResultFileName;
	fstream Write_SimulationDetailFile;              // �ŧifstream����A�ΨӦs�Ҧ����į�Ѽ�
	string SimulationDetailFileName;

	int UEID = UEnumber;
	int Arrival = arrival_factor;
	SimulationResultFileName = IntToString(UEID) + "_" + IntToString(Arrival) + "_Simulation Result.csv";
	SimulationDetailFileName = IntToString(UEID) + "_" + IntToString(Arrival) + "_Simulation Detail.csv";

	Write_SimulationResultFile.open(SimulationResultFileName, ios::out | ios::app);
	if (Write_SimulationResultFile.fail())
		cout << "�ɮ׵L�k�}��" << endl;
	else
	{
		Write_SimulationResultFile << Result->AverageSystemTime << "," << Result->AvgSystemTime << "," << Result->Rho << endl;
	}

	int exist = 0;
	Write_SimulationDetailFile.open(SimulationDetailFileName, ios::in);
	if (Write_SimulationDetailFile)
	{
		char firstline[200];
		Write_SimulationDetailFile >> firstline;
		char cpline[] = "TotalSchedulePacketNum,TotalDiscardPacketNum,TotalRemainPacketNum,TotalThroughput,AvgThroughput,AvgSystemTime,AvgDelay,AvgTransmissionTime,QueueingMeanSystemTime,AvgPacketLossRatio,Rho\n";
		if (strcmp(firstline, cpline))
			exist = 1;
	}
	Write_SimulationDetailFile.close();

	Write_SimulationDetailFile.open(SimulationDetailFileName, ios::out | ios::app);
	if (Write_SimulationDetailFile.fail())
		cout << "�ɮ׵L�k�}��" << endl;
	else
	{
		if (exist == 0)
			Write_SimulationDetailFile << "TotalSchedulePacketNum,TotalDiscardPacketNum,TotalRemainPacketNum,TotalThroughput,AvgThroughput,AvgSystemTime,AvgDelay,AvgTransmissionTime,QueueingMeanSystemTime,AvgPacketLossRatio,Rho" << endl;
		Write_SimulationDetailFile << Result->TotalSchedulePacketNum << "," << Result->TotalDiscardPacketNum << "," << Result->TotalRemainPacketNum << "," << Result->TotalThroughput << "," << Result->AverageThroughput << "," << Result->AverageSystemTime << "," << Result->AverageDelay << "," << Result->AverageTransmissionTime << "," << Result->AvgSystemTime << "," << Result->PacketLossRatio << "," << Result->Rho << endl;
	}
}

void Buffer_Status(int t, BufferStatus *Queue, UE *UEList, vector <double> *TempPacketArrivalTime, SimulationResult *Result)
{
	//�ݬݨC��UE�b�o��TTI���S����ƨ�
	int TTIPacketCount[UEnumber] = { 0 };				//�p��C��UE�b�C��TTI�ɨӪ�packet�Ӽ�
	int TempPacketArrivalTimeID = 0;					//�ݤw�g����o��UE���ĴX��PAT�F
	for (int i = 0; i < UEnumber; i++)
	{
		TTIPacketCount[i] = 0;
		TempPacketArrivalTimeID = Queue->TempPacketArrivalTimeIndex[i];
		if (!TempPacketArrivalTime[i].empty() && (TempPacketArrivalTimeID < TempPacketArrivalTime[i].size()))
		{
			while (TempPacketArrivalTime[i][TempPacketArrivalTimeID] < t + 1)
			{
				Queue->PacketArrivalTime[i].push_back(TempPacketArrivalTime[i][TempPacketArrivalTimeID]);
				TempPacketArrivalTimeID += 1;
				TTIPacketCount[i] = TTIPacketCount[i] + 1;             // �֭p��TTI��packet��
				if (TempPacketArrivalTime[i].empty() || (TempPacketArrivalTimeID > (TempPacketArrivalTime[i].size() - 1)))
					break;
			}
		}
		Result->TotalPacketNum[i] = Result->TotalPacketNum[i] + TTIPacketCount[i];           // ��ثeTTI���Ӫ�packet�ƥ[���`packet��
		Queue->TempPacketArrivalTimeIndex[i] = TempPacketArrivalTimeID;
	}

	// �p�⦹��TTI�C��UE��buffer�̨C��packet��HOL delay
	//for (int i = 0; i<UEnumber; i++)
	//{
	//	Queue->PacketHOLDelay[i].clear();
	//	for (int j = 0; j < Queue->PacketArrivalTime[i].size(); j++)
	//		Queue->PacketHOLDelay[i].push_back((t + 1) - Queue->PacketArrivalTime[i][j]);
	//}

	// �B�zHOL delay�O�_�W�Ldelay budget�A�W�L��packet�ndiscard��
	//for (int i = 0; i<UEnumber; i++)
	//{
	//	//cout << PacketHOLDelay[i].size() << endl;

	//	if (!Queue->PacketHOLDelay[i].empty())									//���ˬdbuffer�̬O�_��packet�ncheck��HOL delay���L�W�Lbudget
	//		while (Queue->PacketHOLDelay[i][0] > UEList[i].delay_budget)			//packet��HOL delay�O�_�W�Ldelay budget
	//		{
	//			//cout << PacketHOLDelay[i][0] << endl;
	//			if (Queue->HeadPacketSize[i] < UEList[i].packet_size)
	//			{
	//				Result->DiscardIncompletePacketNum[i] = Result->DiscardIncompletePacketNum[i] + 1;		//�Ψӭp��Q�屼�����㪺packet��
	//				///Result->TransmissionTime[i] = Result->TransmissionTime[i] + (UEList[i].packet_size - Queue->HeadPacketSize[i]) / resource_element * CQIEfficiency(UEList[i].CQI);
	//			}					

	//			Queue->PacketHOLDelay[i].erase(Queue->PacketHOLDelay[i].begin());							//�]��packet��HOL delay�W�Ldelay budget�A�ҥH�n�屼�Ĥ@��packet
	//			Result->DiscardPacketNum[i] = Result->DiscardPacketNum[i] + 1;								//�֭pdiscard����packet��
	//			//Result->SystemTime[i] = Result->SystemTime[i] + (t + 1) - Queue->PacketArrivalTime[i][0];	//�Qdiscard����packet�b�t�Ϊ��ɶ�
	//			Queue->PacketArrivalTime[i].erase(Queue->PacketArrivalTime[i].begin());						//�]�R�����bPacketArrivalTime�̰O����arrival time
	//			if (Queue->PacketHOLDelay[i].empty())
	//				break;
	//		}

	//	if (!Queue->PacketArrivalTime[i].empty())									// �p��C��UE�bpacket discard���᪺buffer�̦��h�ָ�ƶq
	//	{
	//		Queue->Buffer[i] = (Queue->PacketArrivalTime[i].size() - 1) * UEList[i].packet_size + Queue->HeadPacketSize[i];
	//		Queue->BeforeScheduleBuffer[i] = Queue->Buffer[i];
	//	}
	//	else
	//	{
	//		Queue->Buffer[i] = 0.0;
	//		Queue->BeforeScheduleBuffer[i] = Queue->Buffer[i];
	//	}
	//}
}

bool CP_PacketArrivalTime(Packet a, Packet b) { return a.ArrivalTime < b.ArrivalTime; }

void EqualRB(int t, BufferStatus *Queue, UE *UE, SimulationResult *Result)
{
	int NumRBAssigned[UEnumber] = { 0 };
	double NumBitsTransmited[UEnumber] = { 0 };
	int AssignedUE = 0;								// ����UE��o�FRB

//	vector <double> BuffrtPacketArrivalTime;
//	vector <int> BuffrtPacketUEOrder;

	// �����n�v��RB��UE��T (UE�s���BArrival time of HOL packet)
	vector <Packet> ScheduleUE;

	//int NumUEHaveBufferPacket = 0;				//�o��TTI���ʥ]�n�ǰe��UE�Ӽ�
	//int NumBufferPacket = 0;

	for (int j = 0; j < UEnumber; j++)
	{
		if (Queue->PacketArrivalTime[j].size() != 0)
		{
			Packet temp(j, Queue->PacketArrivalTime[j][0]);
			ScheduleUE.push_back(temp);
		}
	}
	if (ScheduleUE.size() == 0)
		return;

	sort(ScheduleUE.begin(), ScheduleUE.end(), CP_PacketArrivalTime);	//�̷�HOL Packet Arrival Time�q�����Ƨ�

	// �}�l�v��RB�ݭn���t������UE
	for (int i = 0; i < total_RBG; i++)
	{
		if (ScheduleUE.size() == 0)
			return;

		//���tRBi��UE
		AssignedUE = ScheduleUE[i % ScheduleUE.size()].belongUE;
		NumRBAssigned[AssignedUE] += 1;
		double RBCarryBit = resource_element * CQIEfficiency(UE[AssignedUE].CQI);		//�����o�o��RB��UE�ARB�i��a�h�ָ�ƶq

		//�}�l���ƱqUE��buffer�̸˶iRB��
		double RBSizeSpace = RBCarryBit;
		int RBAssign = 1;
		while (RBAssign)
		{
			//�Ĥ@��packet size��RB�i��a����ƶq�j
			if (Queue->HeadPacketSize[AssignedUE] > RBSizeSpace)
			{
				NumBitsTransmited[AssignedUE] += RBSizeSpace;		// �֭p�b�o��TTI�e����ƶq
				Result->Throughput[AssignedUE] += RBSizeSpace;		// �֭pUE��throughput
				Queue->HeadPacketSize[AssignedUE] -= RBSizeSpace;
				RBSizeSpace = 0;
				RBAssign = 0;
			}
			//�Ĥ@��packet size��RB�i��a����ƶq�p
			else
			{
				RBSizeSpace -= Queue->HeadPacketSize[AssignedUE];
				Result->SchedulePacketNum[AssignedUE] += 1;
				NumBitsTransmited[AssignedUE] += Queue->HeadPacketSize[AssignedUE];		// �֭p�b�o��TTI�e����ƶq
				Result->Throughput[AssignedUE] += Queue->HeadPacketSize[AssignedUE];	// �֭pUE��throughput

				double SystemTimeNow = (t + 1) - Queue->PacketArrivalTime[AssignedUE][0];
				if (NumRBAssigned[AssignedUE] > 1)
					SystemTimeNow += 1;
				else
					SystemTimeNow += NumBitsTransmited[AssignedUE] / NumRBAssigned[AssignedUE] * RBCarryBit;
				double SystemTimeHistory = Result->SystemTime[AssignedUE];
				double K = Result->SchedulePacketNum[AssignedUE] - 1;
				double K1 = Result->SchedulePacketNum[AssignedUE];
				Result->SystemTime[AssignedUE] = ((K / K1) * SystemTimeHistory) + ((1 / K1) * SystemTimeNow);	// �p��C�@��packet delay
				Result->Delay[AssignedUE] = ((K / K1) * SystemTimeHistory) + ((1 / K1) * SystemTimeNow);		// �P�W

				Queue->PacketArrivalTime[AssignedUE].erase(Queue->PacketArrivalTime[AssignedUE].begin());
//				Queue->PacketHOLDelay[AssignedUE].erase(Queue->PacketHOLDelay[AssignedUE].begin());
				if (Queue->PacketArrivalTime[AssignedUE].size() == 0)										// �p�G�S����ƭn�ǰe�N�h�X�v��
					ScheduleUE.erase(ScheduleUE.begin() + (i % ScheduleUE.size()));
				if (Queue->PacketArrivalTime[AssignedUE].empty())											// �p�GRBAssign=1, ����٦���L��ƭn�ǡA�ҥH�b�]�@���j��A���ն뺡�o��RB
					RBAssign = 0;																			// �o��RB�p�G�ˤ����]���൹�O�H�ΡA�ҥH�p�G�S����ƭn�ǴN�����o��RB���˽c
				Queue->HeadPacketSize[AssignedUE] = UE[AssignedUE].packet_size;
			}
		}

	}

	// �M��b�o��TTI�`�@���ɶ���O
	//for (int i = 0; i < UEnumber; i++)
	//{
	//	if (NumBitsTransmited[i] == 0)
	//		continue;
	//	double capacity = resource_element * CQIEfficiency(UEList[i].CQI) * NumRBAssigned[i];
	//	Result->TransmissionTime[i] += (NumBitsTransmited[i] / capacity);
	//	Result->SystemTime[i] += (NumBitsTransmited[i] / capacity);
	//}

	// �p��C��UE�b�oTTI scheduling�᪺buffer�̦��h�ָ�ƶq
	//for (int i = 0; i<UEnumber; i++)
	//{
	//	if (!Queue->PacketArrivalTime[i].empty())
	//		Queue->Buffer[i] = (Queue->PacketArrivalTime[i].size() - 1) * UE[i].packet_size + Queue->HeadPacketSize[i];
	//	else
	//		Queue->Buffer[i] = 0;
	//}
}

int main()
{
	for (int times = 0; times < 10; times++)
	{
		cout << "�� " << times << " ��" << endl;
		for (int i = 0; i < UEnumber; i++)
			TempPacketArrivalTime[i].clear();

		for (int i = 0; i < UEnumber; i++)
		{
			//Traffic request initial
			UEList[i].bit_rate = 300;
			UEList[i].packet_size = 8000;
			UEList[i].lambdai = (UEList[i].bit_rate / UEList[i].packet_size) * arrival_factor;
			if (i < UEnumber *0.33)
			{
				DB50_UEnumber++;
				UEList[i].delay_budget = 50;
			}
			else
			{
				if (i < UEnumber *0.66)
				{
					DB100_UEnumber++;
					UEList[i].delay_budget = 100;
				}
				else
				{
					DB300_UEnumber++;
					UEList[i].delay_budget = 300;
				}
			}
		}

		//Coordiante initial
		double utilization = 0.0;
		do
		{
			for (int i = 0; i < UEnumber; i++)
			{
				uniformdistribution(&UEList[i]);
				UEList[i].CQI = getCQI(&UEList[i]);
			}

			//Queueing model calculation
			double Xj = 0.0;
			double lambda = 0.0;
			for (int i = 0; i < UEnumber; i++)
				lambda = lambda + UEList[i].lambdai;
			for (int i = 0; i < UEnumber; i++)
			{
				double weight_i = UEList[i].lambdai / lambda;
				double Xij = UEList[i].packet_size / (resource_element * CQIEfficiency(UEList[i].CQI) * total_RBG);
				Xj += (Xij * weight_i);
			}
			utilization = Xj * lambda;
		} while (utilization > 1);

		cout << utilization << endl;

		//give packet arrival time
		srand((unsigned)time(NULL));			//�üƺؤl
		double BufferTimer = 0.0;				//�C��UE�beNB�̹���buffer���ɶ��b
		double InterArrivalTime = 0.0;			//packet��inter-arrival time
		int AcrossTTI = 0;						//�ΨӧP�_UE���ɶ��b�Apacket��inter-arrival time���L��L��TTI
		for (int i = 0; i < UEnumber; i++)
		{
			//cout << "UE" << i << endl;
			BufferTimer = 0.0;
			AcrossTTI = 0;

			if (outputPAT == 1)
			{
				//�O���C��UE��PAT
				string FileName;									//�ɮצW��
				string UEIndex = IntToString(i);
				FileName = "UE" + UEIndex + "_PAT.txt";				//PAT=packet arrival time
				fstream WriteFile;									//�ŧifstream����
				WriteFile.setf(ios::fixed, ios::floatfield);
				WriteFile.precision(3);
				WriteFile.open(FileName, ios::out | ios::trunc);
				if (WriteFile.fail())
					cout << "�ɮ׵L�k�}��" << endl;
				else
				{
					for (int t = 0; t < simulation_time; t++)
					{
						int TTIPacketCount = 0;

						//�p��C��packet����Ӯɶ��I�A�ðO������TTI�C��UE��buffer�q
						while (BufferTimer <= t + 1)												//�Ψӭp�⦹TTI�ӤF�X��packet�M��TTI�����ɥثebuffer�̪���ƶq
						{
							if (AcrossTTI)															//AcrossTTI = 1��inter arrival time����L��TTI; AcrossTTI=0���L
								WriteFile << BufferTimer << endl;									//�O���C��packet��arrival time
							else
							{
								InterArrivalTime = exponentially_Distributed(UEList[i].lambdai);	//�üƲ���inter-arrival time
								BufferTimer = BufferTimer + InterArrivalTime;						//�����C��UE���ɶ��b
							}
							if (BufferTimer > t + 1)												//BufferTimer���L�W�L�ثe��TTI
							{
								AcrossTTI = 1;
								break;
							}
							else
								if (AcrossTTI)
									AcrossTTI = 0;
								else
									WriteFile << BufferTimer << endl;								// �O���C��packet��arrival time
																									//cout << "Packet arrival time�G" << BufferTimer << endl;
						}
					}
				}
				WriteFile.close();
			}
			else
			{
				for (int t = 0; t < simulation_time; t++)
				{
					int TTIPacketCount = 0;
					//�p��C��packet����Ӯɶ��I�A�ðO������TTI�C��UE��buffer�q
					while (BufferTimer <= t + 1)												//�Ψӭp�⦹TTI�ӤF�X��packet�M��TTI�����ɥثebuffer�̪���ƶq
					{
						if (AcrossTTI)															//AcrossTTI = 1��inter arrival time����L��TTI; AcrossTTI=0���L
							TempPacketArrivalTime[i].push_back(BufferTimer);					//�O���C��packet��arrival time
						else
						{
							InterArrivalTime = exponentially_Distributed(UEList[i].lambdai);	//�üƲ���inter-arrival time
							BufferTimer = BufferTimer + InterArrivalTime;						//�����C��UE���ɶ��b
						}
						if (BufferTimer > t + 1)												//BufferTimer���L�W�L�ثe��TTI
						{
							AcrossTTI = 1;
							break;
						}
						else
							if (AcrossTTI)
								AcrossTTI = 0;
							else
								TempPacketArrivalTime[i].push_back(BufferTimer);				//�O���C��packet��arrival time
																								//cout << "Packet arrival time�G" << BufferTimer << endl;
					}
				}
			}
		}
		cout << "Give PAT end." << endl;

		//�q�~��Ū��PAT
		if (outputPAT == 1)
		{
			//Ū���Ҧ�UE��PAT���Ȧs�_��
			string UEPacketPatternFileName;						//UE packet pattern���ɮצW��
			fstream ReadUEPAT;									//�ŧifstream����
			char UEPacketArrivalTime[200];						//�ΨӦ��stxt�C�@�檺���
			double ArrivalTime = 0.0;							//�ΨӦ��s��X�ӨC�@�檺���
			int NumUETemp = UEnumber;
			string NumUEIndex = IntToString(NumUETemp);
			for (int i = 0; i < UEnumber; i++)
			{
				string UEIndex = IntToString(i);
				UEPacketPatternFileName = "UE" + UEIndex + "_PAT.txt";
				ReadUEPAT.open(UEPacketPatternFileName, ios::in);
				if (!ReadUEPAT)
					cout << "�ɮ׵L�k�}��" << endl;
				else
				{
					while (ReadUEPAT >> UEPacketArrivalTime)
					{
						ArrivalTime = atof(UEPacketArrivalTime);
						TempPacketArrivalTime[i].push_back(ArrivalTime);
					}
				}
				ReadUEPAT.close();
			}
		}

		//Simulation start
		BufferStatus EqualRB_Buffer;
		BufferStatus EqualRB_Buffer_aft100wTTI;
		SimulationResult EqualRB_Result;
		SimulationResult EqualRB_Result_aft1000wTTI;
		for (int i = 0; i < UEnumber; i++)
		{
			EqualRB_Buffer.PacketArrivalTime[i].clear();
			EqualRB_Buffer.PacketHOLDelay[i].clear();
			EqualRB_Buffer.HeadPacketSize[i] = UEList[i].packet_size;
		}

		for (int t = 0; t < simulation_time; t++)
		{
			if ((t + 1) % (simulation_time / 20) == 0)
				cout << (double)(t + 1) / (double)simulation_time * 100 << "%, TTI=" << t + 1 << endl;

			Buffer_Status(t, &EqualRB_Buffer, UEList, TempPacketArrivalTime, &EqualRB_Result);
			EqualRB(t, &EqualRB_Buffer, UEList, &EqualRB_Result);

			if (t == 10000000)
			{
				for (int i = 0; i < UEnumber; i++)
					EqualRB_Result.RemainPacketNum[i] = EqualRB_Buffer.PacketArrivalTime[i].size();
				Simulation_Result(UEList, &EqualRB_Result);
				EqualRB_Result = EqualRB_Result_aft1000wTTI;
			}
		}

		for (int i = 0; i < UEnumber; i++)
			EqualRB_Result.RemainPacketNum[i] = EqualRB_Buffer.PacketArrivalTime[i].size();

		Simulation_Result(UEList, &EqualRB_Result);
	}
}