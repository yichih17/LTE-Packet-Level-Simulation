#ifndef _DEFIEN_H
#define _DEFIEN_H

//Scenario patameter
#define power_eNB 46.0			//dBm
#define radius_eNB 1732			//m
#define total_RBG 100			//Number of total RBG (20Mhz=100)
#define	total_symbol 14			//Number of symbol in a RB (14)
#define ctrl_symbol 2			//Number of symbol for control signal in a RB (1~3)
#define subcarrier 12			//Number of subcarrier (12)
#define resource_element (total_symbol - ctrl_symbol) * subcarrier	//Number of resource element per RB for data

#define power_AP  23.0			//dBm

#define rho_max 0.99			//Max load of BSs

//Algoritm parameter
#define depth_max 2
#define satisfied_TH 75

//Simulation parameter
#define simulation_time 100000000	//ms(TTI
#define UE_dis_mode 1			//0: uniform 1:hotspot
#define UE_type_number 3		//DB = 50, 100, 300ms
#define UEnumber 25
#define arrival_factor 1

#include<vector>

enum type_bs { macro, ap, ue };
enum type_ue { type1, type2, type3, type4 };
enum type_distribution { uniform, hotspot };

struct BS;
struct UE;
extern std::vector <BS> vbslist;
extern std::vector <UE> vuelist;

int LTECQIRange[];
int range_ap[];
extern double macro_eff[15];
extern double ap_capacity[8];

/*attribute of BS*/
struct BS
{
	int num;
	type_bs type;
	double coor_X, coor_Y;
	double lambda;
	double systemT;
	double systemT_constraint;
	std::vector <UE*> connectingUE;
};

/*attribute of UE*/
struct UE
{
	int num;
	type_ue type;
	double coor_X, coor_Y;
	double lambdai;
	int delay_budget;
	double packet_size;
	double bit_rate;
	BS* connecting_BS;
	int CQI;
	std::vector <BS*> availBS;
};

struct result
{
	int outage_number;
};

struct connection_status
{
	std::vector <BS> bslist;
	std::vector <UE> uelist;
	int influence;
	int outage_dso;
};

struct hotspot
{
	double coor_x;
	double coor_y;
};

struct Packet
{
	int belongUE;
	double ArrivalTime;

	Packet(int n, double t)
	{
		belongUE = n;
		ArrivalTime = t;
	}
};

class BufferStatus
{
public:
	int TempPacketArrivalTimeIndex[UEnumber];       // �ΨӬ����bTempPacketArrivalTime�o��vector����ĴX��packet�F
	std::vector <double> PacketArrivalTime[UEnumber];     // packet��arrival time(ms)
	std::vector <double> PacketHOLDelay[UEnumber];        // packet��HOL delay(ms)
	double Buffer[UEnumber];                        // �C��UE�beNB�̹���buffer����ƶq(bits)
	double BeforeScheduleBuffer[UEnumber];          // �Ƶ{�ebuffer���q
	double AfterScheduleBuffer[UEnumber];           // �Ƶ{��buffer���q
	double HeadPacketSize[UEnumber];                // �C��UE buffer�̪��Ĥ@��packet size���h�j(bits)
	BufferStatus()                               // ��l��class���ܼ�
	{
		memset(TempPacketArrivalTimeIndex, 0, sizeof(TempPacketArrivalTimeIndex));
		memset(Buffer, 0, sizeof(Buffer));
		memset(BeforeScheduleBuffer, 0, sizeof(BeforeScheduleBuffer));
		memset(AfterScheduleBuffer, 0, sizeof(AfterScheduleBuffer));
		memset(HeadPacketSize, 0, sizeof(HeadPacketSize));
	}
};

class SimulationResult
{
public:
	double PastDataAmount[UEnumber];				// �C��UE�bt�ɹL�h���쪺��ƶq
	double AccumulateRate[UEnumber];				// �L�h��ӫ��ƥ���rate���v          
	int TotalPacketNum[UEnumber];					// �C��UE�beNB�̹���buffer���Ӫ��`packete�Ӽ�
	int DiscardIncompletePacketNum[UEnumber];		// �Ψӭp��Q�屼�������㪺packet���X��
	int DiscardPacketNum[UEnumber];					// �C��UE�beNB�̹���buffer��discard��packet��
	int SchedulePacketNum[UEnumber];				// �C��UE�beNB�̹���buffer��schedule��packet��
	int RemainPacketNum[UEnumber];					// �{��������C��UE buffer�Ѿl��Packet�q
													//double PacketDelay[UEnumber];					// �C��UE�����ʥ]delay
	double Throughput[UEnumber];					// �C��UE��throughput
	double Delay[UEnumber];							// �C��UE���ʥ]�bbuffer�̵��ݪ��ɶ�
	double SystemTime[UEnumber];					// �C��UE���ʥ]�q�i�Jbuffer��ǰe�������ɶ�
	double TransmissionTime[UEnumber];
	double RateSatisfaction[UEnumber];				// �C��UE��rate satisfaction
	double DelaySatisfaction[UEnumber];				// �C��UE��delay satisfaction
	double AvgSystemTime;						// Queueing��X�Ӫ�
	double Rho;
	double TotalThroughput;						// �ΨӰO������t�Ϊ�throughput
	int TotalSchedulePacketNum;					// �ΨӰO������t�αƵ{�h��packet
	int TotalDiscardPacketNum;					// �ΨӰO������t�ά�h��packet
	int TotalRemainPacketNum;
	double AverageThroughput;					// �ΨӰO���Ҧ�UE������throughput
	double AverageDelay;						// �ΨӰO���Ҧ�UE������delay
	double AverageTransmissionTime;
	double AverageSystemTime;
	double PacketLossRatio;						// �ΨӰO������t�Ϊ�packet loss ratio	
	double Type1_TotalThroughput;				// �ΨӰO��type1 UE��throughput
	int Type1_SchedulePacketNum;				// �ΨӰO��type1 UE�Ƶ{�h��packet
	int Type1_DiscardPacketNum;					// �ΨӰO��type1 UE��h��packet
	double Type1_AverageThroughput;				// �ΨӰO��type1 UE������throughput
	double Type1_AverageDelay;					// �ΨӰO��type1 UE������delay
	double Type1_PacketLossRatio;				// �ΨӰO��type1 UE��packet loss ratio
	double Type2_TotalThroughput;				// �ΨӰO��type2 UE��throughput
	int Type2_SchedulePacketNum;				// �ΨӰO��type2 UE�Ƶ{�h��packet
	int Type2_DiscardPacketNum;					// �ΨӰO��type2 UE��h��packet	
	double Type2_AverageThroughput;				// �ΨӰO��type2 UE������throughput
	double Type2_AverageDelay;					// �ΨӰO��type2 UE������delay
	double Type2_PacketLossRatio;				// �ΨӰO��type2 UE��packet loss ratio
	double Type3_TotalThroughput;				// �ΨӰO��type3 UE��throughput
	int Type3_SchedulePacketNum;				// �ΨӰO��type3 UE�Ƶ{�h��packet
	int Type3_DiscardPacketNum;					// �ΨӰO��type3 UE��h��packet	
	double Type3_AverageThroughput;				// �ΨӰO��type3 UE������throughput
	double Type3_AverageDelay;					// �ΨӰO��type3 UE������delay
	double Type3_PacketLossRatio;				// �ΨӰO��type3 UE��packet loss ratio
	double Type4_TotalThroughput;				// �ΨӰO��type4 UE��throughput
	int Type4_SchedulePacketNum;				// �ΨӰO��type4 UE�Ƶ{�h��packet
	int Type4_DiscardPacketNum;					// �ΨӰO��type4 UE��h��packet	
	double Type4_AverageThroughput;				// �ΨӰO��type4 UE������throughput
	double Type4_AverageDelay;					// �ΨӰO��type4 UE������delay
	double Type4_PacketLossRatio;				// �ΨӰO��type4 UE��packet loss ratio
	SimulationResult()						// ��l��class���ܼ�
	{
		memset(PastDataAmount, 0, sizeof(PastDataAmount));
		memset(AccumulateRate, 1, sizeof(AccumulateRate));
		memset(TotalPacketNum, 0, sizeof(TotalPacketNum));
		memset(DiscardIncompletePacketNum, 0, sizeof(DiscardIncompletePacketNum));
		memset(DiscardPacketNum, 0, sizeof(DiscardPacketNum));
		memset(SchedulePacketNum, 0, sizeof(SchedulePacketNum));
		memset(RemainPacketNum, 0, sizeof(RemainPacketNum));
		memset(Throughput, 0, sizeof(Throughput));
		memset(Delay, 0, sizeof(Delay));
		memset(SystemTime, 0, sizeof(SystemTime));
		memset(TransmissionTime, 0, sizeof(TransmissionTime));
		memset(RateSatisfaction, 0, sizeof(RateSatisfaction));
		memset(DelaySatisfaction, 0, sizeof(DelaySatisfaction));
		AvgSystemTime = 0.0;
		Rho = 0.0;
		TotalThroughput = 0.0;
		TotalSchedulePacketNum = 0;
		TotalDiscardPacketNum = 0;
		TotalRemainPacketNum = 0;
		AverageThroughput = 0.0;
		AverageDelay = 0.0;
		AverageTransmissionTime = 0.0;
		AverageSystemTime = 0.0;
		PacketLossRatio = 0.0;
		Type1_TotalThroughput = 0.0;
		Type1_SchedulePacketNum = 0;
		Type1_DiscardPacketNum = 0;
		Type1_AverageThroughput = 0.0;
		Type1_AverageDelay = 0.0;
		Type1_PacketLossRatio = 0.0;
		Type2_TotalThroughput = 0.0;
		Type2_SchedulePacketNum = 0;
		Type2_DiscardPacketNum = 0;
		Type2_AverageThroughput = 0.0;
		Type2_AverageDelay = 0.0;
		Type2_PacketLossRatio = 0.0;
		Type3_TotalThroughput = 0.0;
		Type3_SchedulePacketNum = 0;
		Type3_DiscardPacketNum = 0;
		Type3_AverageThroughput = 0.0;
		Type3_AverageDelay = 0.0;
		Type3_PacketLossRatio = 0.0;
		Type4_TotalThroughput = 0.0;
		Type4_SchedulePacketNum = 0;
		Type4_DiscardPacketNum = 0;
		Type4_AverageThroughput = 0.0;
		Type4_AverageDelay = 0.0;
		Type4_PacketLossRatio = 0.0;
	}
};

#endif // !_DEFIEN_H