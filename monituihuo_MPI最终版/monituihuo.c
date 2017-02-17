#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include <sys/time.h>
#include <mpi.h>


#define ZUIDADIANSHU 100	       //���ĵ���
#define QISHIDIAN 4	              //��ʼ��
#define CHUSHIWENDU 1400000	      //��ʼ�¶�
#define WENDUXISHU 0.99	             //����ϵ��
#define T_MIN 0.000001              //��Ҫ�����������

//ȫ�ֱ���
char point_list[ZUIDADIANSHU][20];			//�洢������
double point_coordinate[ZUIDADIANSHU][2];	      //�洢��x y����

double P(int *i,int *j,double t,int point_number);        //�Ƚ�����·�����У�������i>j,����1.0������ʹ��m׼��
double distance_sum(int *x,int point_number);             //�Ż�·������x��·������
void Neighbour(int *father,int *result,int point_number); //�����µ�����
double distance(int x,int y);                             //����������ľ��볤��
double random0_1(void);                                   //���������,��Χ��0-1֮��
int *random2(int point_number);                           //����һ����0��ͷ�� 0-point_number�Ĳ��ظ�����
double mytime();                                          //��z�ص�ǰʱ��

int main()
{

	int my_rank,comm_sz;              //��ȡ���̺�
	double tstart,tstop;              //��ʼ�ͽ���ʱ��
	char filename[81];                //�ļ���
	double x_position,y_position;    //���x��y����
	FILE *pfile;                    //�ļ�
	int *sequence;                //��ʼ����
	int point_number;	     //������  ���Կ����ú궨�嶨���������
	int i2;			    //forѭ��ר�ñ���
	int *endlist;              //���·������
	double sumlen;             //���·������
	int **buf2;               //�ռ��������̵ľֲ����Ž�
	int *buf ;              //�ռ��������̵ľֲ����Ž�
	int	*temp ;         //���ڽ���
	double random ;         //����·�����еıȽϽ��
	int *local_i ;          //��ǰ���Ž�
	int *local_j ;          //�µ�·������
	int c,m,n,k;
	double local_len,nextlen,len,d_len;
 	double t , rate; //��ʼ�¶ȣ�����ϵ��
   		              

  MPI_Init(NULL,NULL);               //��ʼ��
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank); //��ȡ���̺�
  MPI_Comm_size(MPI_COMM_WORLD,&comm_sz);

	tstart=mytime();
	strcpy(filename,"100.txt");

    if((pfile = fopen(filename,"r"))==NULL)
    {
	  printf("Not find %s!\n",filename);
	  return 0;
    }

     /*��ȡĿ���ļ�������Ϣ*/
    fscanf(pfile,"%d",&point_number);
    if(my_rank==0){
	  printf("numbers 0f city are=%d\n",point_number);
    }

	 for(i2=0;i2<point_number;i2++)
	{
		fscanf(pfile,"%s\t%lf\t%lf",&point_list[i2],&x_position,&y_position);
		point_coordinate[i2][0] = x_position;
		point_coordinate[i2][1] = y_position;
	}

	fclose(pfile);
	srand( (unsigned)time( NULL ) );    //�������� ��ʼ�����������
	sequence = random2(point_number);

  // MPI_Barrier(MPI_COMM_WORLD);

  /*һЩ������ֵ */
	random = 0;
	local_i = malloc(point_number*sizeof(int));
	local_j = malloc(point_number*sizeof(int));
    nextlen=0.0;len=0.0;d_len;
 	t = CHUSHIWENDU;rate = WENDUXISHU;

	for(m=0;m<point_number;m++)
	{
		local_i[m] = sequence[m];
		local_j[m] = 0;
	}

	MPI_Barrier(MPI_COMM_WORLD);

	while(t>T_MIN)
	{
	  do
	  {
		for(c=0;c<20000/comm_sz;c++)
		{
		  Neighbour(local_i,local_j,point_number);
				random = P(local_i,local_j,t,point_number);
				if ( (random == 1.0) || (random > random0_1()) )
				 {
					temp = local_i;
					local_i = local_j;
					local_j = temp;  //��֤local_i����ʼ���ǵ�ǰ��ʼ����
				 }
		}

		local_len=distance_sum(local_i,point_number);
		if(my_rank==0) nextlen=len;
		MPI_Barrier(MPI_COMM_WORLD);

		MPI_Reduce(&local_len,&len,1,MPI_DOUBLE,MPI_MIN,0,MPI_COMM_WORLD);  //�ۼ�������ѡȡ��С�ĸ���len

		if(my_rank==0)
			d_len=len-nextlen;

		MPI_Bcast(&d_len,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	   }while(d_len>0.1);

	   t*=rate;
	}

	//MPI_Barrier(MPI_COMM_WORLD);
	 /* һЩ������ֵ*/
	if(my_rank==0)
	{
	   buf2 = malloc(comm_sz*sizeof(int*));
	   for(n=0;n<comm_sz;n++)
	   {
		buf2[n]=(int *)malloc(point_number*sizeof(int));
	   }
	   buf = (int *)malloc(comm_sz*point_number*sizeof(int));

	}

	 //MPI_Barrier(MPI_COMM_WORLD);
	 MPI_Gather(local_i,point_number,MPI_INT,buf,point_number,MPI_INT,0,MPI_COMM_WORLD);    //�Ѹ������̵ľֲ����Ž��ռ�����

	  /*�ڸ����ֲ����Ž���ѡ�����Ž�*/
	 if(my_rank==0)
	 {
	   for(n=0;n<comm_sz;n++)
	   {
			for(k=0;k<point_number;k++)
			{
			  buf2[n][k]=buf[n*point_number+k];
			}
	   }

	   for(n=1;n<comm_sz;n++)
	   {
		 random = P(buf2[0],buf2[n],t,point_number);
			 if ( (random == 1.0) )
			 {
				temp = buf2[0];
				buf2[0] = buf2[n];
				buf2[n] = temp;    //��֤buf2[0]����ʼ���ǵ�ǰ��ʼ����
			 }
	   }

	   endlist=buf2[0];
	   printf("last: ");

	   for(i2=0;i2<point_number;i2++)
	   {
			printf("%d ",endlist[i2]);
	   }

	   sumlen = distance_sum(endlist,point_number);
	   printf("\n sumlen :%lf\n",sumlen);
	   free(endlist);
	   free(sequence);
	   tstop=mytime();
	   printf("spend time=%lf\n",tstop-tstart);
	 }

	  MPI_Finalize();
	  return 0;
	}


//����ʱ��ĺ���
double mytime()
{
    double ts = 0.0;
    struct timeval mt;
    gettimeofday(&mt,(struct timezone*)0);
    ts = (double)(mt.tv_sec+mt.tv_usec*1.0e-6);
    return (ts);
}


/*����һ����0��ͷ�� 0-point_number�Ĳ��ظ�����*/
int *random2(int point_number)
{
	int temp = 0,signal = 1,k=0;
	int i,j;
	int *save = NULL;
	save =  (int *)malloc(point_number*sizeof(int));

	//�����ʼ��
	for(j = 0;j < point_number;j++)
		save[j] = -1;

	/*����һ��0-point_number�Ĳ��ظ�����*/
	do
	{
		signal = 1;
		temp = rand() % point_number;	//tempȡ0-point_number��ֵ
		for(i=0;i<point_number;i++)
		{
			if(save[i] == temp)
			{
				signal = 0;
				break;
			}
		}
		if(signal != 0)
		{
			save[k] = temp;
			k++;
		}
	}while(signal == 0 || k != point_number);

	/*ʹ��save[0]=0��save[0]Ϊ��ʼ�㣩*/
	for(i=1;i<point_number;i++)
	{
		if(save[i]==QISHIDIAN)
		{
			temp = save[i];
			save[i] = save[0];
			save[0] = temp;
		}
	}
	return save;
}

/*���������,��Χ��0-1֮��*/
double random0_1(void)
{
	return (double)rand() / (double)RAND_MAX;
}


/*����x y 2��֮��ľ���*/
double distance(int x,int y)
{
	double xy_distance = 0;
//	printf("x=%d y=%d\t",x,y);
	xy_distance = sqrt( (point_coordinate[x][0] - point_coordinate[y][0]) * (point_coordinate[x][0] - point_coordinate[y][0]) +
   						(point_coordinate[x][1] - point_coordinate[y][1]) * (point_coordinate[x][1] - point_coordinate[y][1]) );
	return xy_distance;
}


/* ����������
father�������У�result����������,point_number�������� ����������洢��result��*/
void Neighbour(int *father,int *result,int point_number)
{
	int n = 0,m = 0,temp = 0;
	int k;
	for(k=0;k<point_number;k++)
		result[k] = father[k];

	do
	{
		n = rand() % (point_number - 1); //-1-point_number-1
		m = rand() % (point_number - 1);
	}while(n == m);		//�������Ĳ���ȵ�n��m
	n++;
	m++;//0-point_number
	//����
	temp = result[n];
	result[n] = result[m];
	result[m] = temp;
}


/*����ĳһ�������г���֮��ľ����ܺ�*/
double distance_sum(int *x,int point_number)
{
	double result = 0;
	int i;
	for(i=0;i<point_number-1;i++)
	{
		result += distance(x[i],x[i+1]);
	}
//	result += distance(x[point_number-1],x[0]);	//�Ƿ�ص���ʼ��
	return result;
}



//t����ǰ�¶�,i,j�ֱ����ͬ����������,���ض�Ӧ��ת�Ʒ�������
double P(int *i,int *j,double t,int point_number)
{
	double fi = 0,fj = 0;
	double result = 0;
	fi = distance_sum(i,point_number);
	fj = distance_sum(j,point_number);
	if(fj < fi)
		result = 1.0;
	else
		result = exp( (fi - fj)/t );
	return result;
}

