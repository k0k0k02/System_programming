#include<stdio.h>
#include<dirent.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<string.h>
#include"20161588.h"   
unsigned char memory[65536][16]={0,};
hashNode **hashTable;
char symbol_table[30][30]; // symbol을 저장하는 배열
char format[10];	// opcode의 format return시 저장 
int symbol_address[30];	// symbol의 위치를 저장하는 배열
int sym_cnt = 0;	// symbol의 개수를 저장하는 변수
int start_address, end_address;	// 시작 주소, 끝 주소를 저장하여 obj file에 사용
int loc_arr[100];	// 각 line의 PC를 저장하는 배열
char start_symbol[30];	// program name을 저장하는 변수
char reg[10][2]={"A","X","L","B","S","T","F"," ","PC","SW"}; //register의 이름을 저장하는 배열
int main()
{
	
    historyNode *listHead = malloc(sizeof(historyNode));
	node_init(listHead);  
    historyNode *listTail = listHead;		// history 추가시 참조하기 위한 꼬리칸
	
	hashTable = (hashNode**)malloc(sizeof(hashNode*)*20);
	for(int i=0;i<20;i++){
		hashTable[i] = malloc(sizeof(hashNode));
	}	
	FILE *fp = fopen("opcode.txt","r");
	int opcode;
	char mnemonic[10];
	char form[10];
	for(int i=0;i<58;i++){
		fscanf(fp,"%x %s %s",&opcode,mnemonic,form);
		hash_push(opcode,mnemonic,form);
	}
	
    char *command = malloc(sizeof(char)*100);
	char *command1 = malloc(sizeof(char)*100);
	int dumpAddress=0;
	while(1){
        printf("sicsim> ");
		fgets(command,100,stdin);
		command = nomalization(command);		// 띄어쓰기 등 예외 처리
		strcpy(command1,command);
		char *token0 = strtok(command1, " ");
		if(!strcmp(command,"help") || !strcmp(command,"h")){	//help
            print_help();
        }
        else if(!strcmp(command,"dir") || !strcmp(command,"d")){	//dir
            print_dir();
        }
        else if(!strcmp(command,"quit") || !strcmp(command,"q")){	//quit
            break;
        }
        else if(!strcmp(command, "history") || !strcmp(command,"hi")){	// history
            listTail = command_push(command,listTail);
            print_history(listHead);
            continue;
        }
		else if(!strcmp(command, "dump") || !strcmp(command,"du")){	//dump 
			print_dump(dumpAddress,dumpAddress+159);	// 160개 출력
			dumpAddress+=160;							// 다음 위치 저장
			if(dumpAddress>MAX_BUFF) dumpAddress=0;		// 범위 넘을 경우 초기화
		}
		else if(!strcmp(token0, "dump") || !strcmp(token0, "du")){	// dump 입력이 여러 개일 경우
			char *token1 = strtok(NULL, ",");
			char *token2 = strtok(NULL, "\n");
			int address1=0,address2=0,errorFlag=0;
			token1 = nomalization(token1);  // 공백 처리
			address1 = hex_check(token1,&errorFlag);
			if(token2 != NULL){	// end가 지정된 경우
				token2 = nomalization(token2);
				address2 = hex_check(token2,&errorFlag);
			}
			else{		//end가 지정되지 않은 경우
				address2 = address1+159;
			}
			if(errorFlag==1 || address1>address2){	// errorFlag : 16진수 이외의 입력이 받아졌을 경우
				printf("input error\n");
				continue;
			}
			dumpAddress= address2+1;	// dump 다음 위치 저장
			if(dumpAddress>MAX_BUFF) dumpAddress=0;	
			print_dump(address1,address2);
		}
		else if(!strcmp(token0, "edit") || !strcmp(token0,"e")){ // edit
			char *token1 = strtok(NULL, ",");
			char *token2 = strtok(NULL, "\n");
			if(token2 == NULL){
				printf("input error\n");
				continue;
			}
			token1 = nomalization(token1);	// 공백 등 예외 처리
			token2 = nomalization(token2);

			int address=0,value=0,errorFlag=0;
			address = hex_check(token1,&errorFlag);
			value = hex_check(token2,&errorFlag);
			if(errorFlag==1 || value>255){	//FF이상의 입력이 받아졌을 때
				printf("input error\n");
				continue;
			}
			memory[address/16][address%16] = value;
		}
		else if(!strcmp(token0, "fill") || !strcmp(token0,"f")){	// fill
			char *token1 = strtok(NULL, ",");
			char *token2 = strtok(NULL, ",");
			char *token3 = strtok(NULL, "\n");
			if(token3 == NULL){
				printf("input error\n");
				continue;
			}
			token1 = nomalization(token1);
			token2 = nomalization(token2);
			token3 = nomalization(token3);

			int start=0,end=0,value=0,errorFlag=0;
			start = hex_check(token1,&errorFlag);
			end = hex_check(token2,&errorFlag);
			value = hex_check(token3,&errorFlag);

			if(errorFlag == 1 || value> 255 || start>end){
				printf("input error\n");
			}
			int i=0,cnt=0;
			do{
				int row = start/16+i;
				for(int j=0;j<16;j++){
					if((i==0 && j<start%16) || end-start<cnt) continue;	//end-start만큼 메모리 처리
					memory[row][j] = value;cnt++;
				}
				i++;
			}while(end-start>=i);
		}
		else if(!strcmp(command,"reset")){	//reset
			memset(memory,0,sizeof(memory));
		}
		else if(!strcmp(token0,"opcode")){	//opcode mnemonic
			char *token1 = strtok(NULL,"\n");
			token1 = nomalization(token1);
			int flag = hash_search(token1,1);	// flag(0): 검색하여 찾지 못한 경우
			if(flag==0) continue;
		}
		else if(!strcmp(command,"opcodelist")){	//opcodelist
			hash_print();
		}
		else if(!strcmp(token0,"type")){		//type 명령시
			char *token1 = strtok(NULL,"\n");
			token1 = nomalization(token1);
			FILE *filename = fopen(token1,"r");
			if(filename == NULL){		// file을 못 찾을 경우
				printf("input error\n");
				continue;
			}
			char buffer[128];
			while(fgets(buffer,128,filename)!=NULL){	// 찾은 경우 내용을 출력
				printf("%s",buffer);
			}
			fclose(filename);	// 닫기
		}
		else if(!strcmp(token0,"assemble")){	//assemble 명령시
			char *token1 = strtok(NULL,"\n");
			token1 = nomalization(token1);
			int asmflag=0;
			asmflag = assembler(token1);	//assembler 함수 호출
			file_remove(token1,asmflag);	//assemble 후 임시 파일삭제 오류시 obj, lst파일도 삭제
			if(asmflag == 0){				//오류시 symbol table 초기화
				for(int i=0;i<sym_cnt;i++) memset(symbol_table[i],'\0',sizeof(symbol_table[i]));
				sym_cnt=0;	
				continue;
			}
		}
		else if(!strcmp(command,"symbol")){	//symbol 명령시
			if(sym_cnt == 0){				//symbol table이 만들어지지 않은 경우		
				printf("Symbol table has not created\n");
				continue;
			}
			for(int i=0;i<sym_cnt;i++){		// 만들어진 경우 출력
				printf("\t%s\t%04X\n",symbol_table[i],symbol_address[i]);
			}
		}
        else{
			printf("input error\n");	// 이외의 입력은 모두 예외 처리
			continue;
		}
        listTail = command_push(command,listTail);	// 예외처리 되지 않은 경우 history에 저장
	}																
	free(listHead);
	free(listTail);
	for(int i=0;i<20;i++) free(hashTable[i]);
	free(hashTable);
	fclose(fp);
	return 0;
}
void file_remove(char* filename,int flag){	// 임시파일 및 오류시 obj, lst파일을 삭제해주는 함수
	remove("intermediate_file");	// 임시파일 삭제
	remove("obj_temp");
	char *obj;
	char lst[20];
	if(flag == 0){		// 오류시 obj, lst파일 삭제 
   		obj = strtok(filename,".");
		strcpy(lst,obj);
		strcat(obj,".obj");
		strcat(lst,".lst");
		remove(obj);
		remove(lst);
	}
}
int assembler(char* filename){	// pass 1, 2 호출하는 함수
	FILE *fp = fopen(filename,"r");
	if(fp == NULL){
		printf("input error\n");
		return 0;
	}
	int passflag = pass1(fp);	//pass1 호출, passflag = 0이면 오류 
	if(passflag == 0) return 0;
	else{
		passflag = pass2(filename);	// pass1에서 오류가 아닌 경우 pass2 호출
		if(passflag == 0) return 0;
	}
	char str_temp[30];
	for(int i=0;i<sym_cnt;i++){		//symbol table 정렬 
		for(int j=i+1;j<sym_cnt;j++){
			if(strcmp(symbol_table[i],symbol_table[j])>0){
				strcpy(str_temp,symbol_table[i]);
				strcpy(symbol_table[i],symbol_table[j]);
				strcpy(symbol_table[j],str_temp);
				int_swap(&symbol_address[i],&symbol_address[j]);
			}
		}
	}
	return 1;
}
void int_swap(int *a, int *b){	// int형 swap 함수
	int temp;
	temp = *a;
	*a = *b;
	*b= temp;
}
int pass1(FILE *filename){	// pass1
	sym_cnt = 0;	
	start_address = 0;	
	int loc_cnt=0;	// locctr
	int line_num=0;  // line number 저장 
	int line_cnt=0;	//loc_arr를 위한 변수
	int token_cnt=0;	// 입력 개수
	char token[3][100]; // line을 쪼갤 token
	char line[100];	// asm 파일을 가져올 변수
	FILE *tempfile = fopen("intermediate_file","w");	// 임시파일 생성
	
	if(tempfile == NULL){
		printf("file making error\n");
		return 0;
	}
	while (1) {
		memset(line,'\0',sizeof(line));
		if(fgets(line,sizeof(line),filename)==NULL) break;	// asm파일이 끝나면 break
		int opcode = 0;
		token_cnt = sscanf(line, "%s %s %s", token[0], token[1], token[2]); // line을 쪼갬
		if(line_num > 0){
			loc_arr[line_cnt++] = loc_cnt;	// 이전 line에 해당하는 배열에 현재 locctr 저장
		}
		line_num += 5;	// line_num 갱신 
		fprintf(tempfile, "%-5d\t", line_num);	// line_number 출력 
		for (int i = 0; i < strlen(token[1]); i++) {	// operand가 2개인 경우 처리
			if (token[1][i] == ',') token_cnt--;
		}
		if(!strcmp(token[0],".")){		// 주석인 경우
			fprintf(tempfile, "%s",line);
			continue;
		}
		else {
			if (token_cnt == 3) {	//symbol, opcode, operand인 경우
				opcode = 1;
				if(strcmp(token[opcode],"START")){	// opcode가 start인 경우 symbol부 저장 x
					for (int i = 0; i < sym_cnt; i++) {	// 해당 symbol이 이미 저장된 경우
						if (!strcmp(symbol_table[i], token[0])) {
							printf("Duplicate symbol error, line: %d\n", line_num / 5);
							return 0;
						}
					}
					strcpy(symbol_table[sym_cnt], token[0]);	// symbol 저장
					symbol_address[sym_cnt] = loc_cnt; // symbol address 저장
					sym_cnt++;	// symbol 개수 증가
				}
			}
			if (!strcmp(token[opcode], "START")) { // opcode start인 경우
				int hexflag = 0;
				loc_cnt = hex_check(token[opcode + 1], &hexflag);	// operand를 10진수로 변환하여 저장
				if (hexflag == 1) {	//16진수이외의 문자가 저장된 경우 오류처리
					printf("Start address error, line: %d\n", line_num / 5);
					return 0;
				}
				strcpy(start_symbol,token[0]);
				start_address = loc_cnt;	// 프로그램 시작 주소 저장
				fprintf(tempfile, "%04X\t", loc_cnt); //임시파일에 출력
				fprintf(tempfile, "%s", line);
			}
			else if (!strcmp(token[opcode], "BASE")) {	//opcode부가 base인 경우
				fprintf(tempfile, "?\t%s", line);	// symbol 부분을 ?처리(pass2에서의 처리를 위해)
			}
			else if (!strcmp(token[opcode], "RSUB")) {	// rsub인 경우
				fprintf(tempfile, "%04X\t", loc_cnt);
					fprintf(tempfile, "%s", line);
					loc_cnt += 3; // rsub은 locctr 3증가 
			}
			else if (!strcmp(token[opcode], "END")) {	//end인 경우
				fprintf(tempfile, "?\t%s", line);	// base와 마찬가지
					break;	//반복문 끝
			}
			else if (!strcmp(token[opcode], "WORD")) {	// word인 경우
				fprintf(tempfile, "%04X\t", loc_cnt);
				fprintf(tempfile, "%s", line);
				loc_cnt += 3;	// word는 locctr 3증가
			}
			else if (!strcmp(token[opcode], "BYTE")) {	//byte인 경우
				fprintf(tempfile, "%04X\t", loc_cnt);
				fprintf(tempfile, "%s", line);
				if (token[2][0] == 'C') loc_cnt += strlen(token[2]) - 3;	// c인 경우, x인 경우에 따라 locctr 처리
				else if (token[2][0] == 'X') loc_cnt += (strlen(token[2]) - 3) / 2;
			}
			else if (!strcmp(token[opcode], "RESW")) {	//resw인 경우
				fprintf(tempfile, "%04X\t", loc_cnt);
				fprintf(tempfile, "%s", line);
				int t = atoi(token[2]);	// operand부를 10진수로 받음
				loc_cnt += t * 3;	// locctr 갱신 
			}
			else if (!strcmp(token[opcode], "RESB")) {	//resb인 경우
				fprintf(tempfile, "%04X\t", loc_cnt);
				fprintf(tempfile, "%s", line);
				int t = atoi(token[2]);	// operand부를 10진수로 받음
				loc_cnt += t;	//locctr 갱신
			}
			else {		// opcode인 경우
				char temp_token[10]={'\0',};
				int plus_flag = 0;
				if (token[opcode][0] == '+') {	// +인 경우, 4형식 처리
					fprintf(tempfile, "%04X\t", loc_cnt);
					fprintf(tempfile, "%s", line);
					loc_cnt += 4;		// 4형식이므로 4 증가
					plus_flag = 1;		// 이후 처리를 위해 flag 처리
					for (int i = 1; i < strlen(token[opcode]); i++) temp_token[i - 1] = token[opcode][i];	// + 문자 삭제
				}
				else strcpy(temp_token, token[opcode]);	// +가 아닌 경우 그대로 저장
				int result = hash_search(temp_token,0);	// opcode table search
				if (result == -1) {	// 못 찾았으면
					printf("Invalid opcode error, line: %d\n",line_num / 5);
					return 0;
				}
				else if (!plus_flag) {	// 찾았고, +가 아닌 경우 (+인 경우에는 처리를 이미 했으므로)
					fprintf(tempfile,"%04X\t",loc_cnt);
					fprintf(tempfile,"%s",line);
					if (!strcmp(format, "1")) loc_cnt++;	// 형식에 따라 locctr 갱신
					else if (!strcmp(format, "2")) loc_cnt += 2;
					else loc_cnt += 3;
					
				}
			}
		}
	}
	end_address = loc_cnt;	// 마지막 주소 저장
	fclose(tempfile);	// 임시파일 닫기
	return 1;
}
int pass2(char *filename){	//pass2
	FILE *tempfile = fopen("intermediate_file","r");	// pass1에서 저장한 임시파일 open
	char temp[20];	// strtok을 위해 filename을 저장할 변수
	char *obj;	// obj파일명 저장할 변수
	char lst[20];	// lst 파일명 저장할 변수
	char line[100];	// 임시파일 line 입력
	int line_num;	// line number
	char loc_cnt[20],symbol[20],opcode[20],operand1[20],operand2[20];
	// line을 쪼깨어 저장할 변수들
	strcpy(temp,filename);
	obj = strtok(temp,".");	// .asm파일 이름만 저장
	strcpy(lst,obj);
	strcat(obj,".obj");	// 뒤에 .obj 저장
	strcat(lst,".lst"); // 뒤에 .lst 저장
    FILE *objfile = fopen(obj,"w");	// obj 파일 생성
	FILE *obj_temp = fopen("obj_temp","w");	// obj임시파일 생성
	FILE *lstfile = fopen(lst,"w");	// lst 파일 생성
	if(tempfile == NULL || objfile == NULL || lstfile == NULL || obj_temp==NULL){
		printf("file open error\n");
		return 0;
	}
	int line_iter=0,i;	// pc 처리를 위해 iter
	int pc=0,cnt=0;	// pc, cnt = line 토큰 개수
	int base=0,type=0;	// base register, format
	int op_number=0;	// opcode 번호
	int mod_address[30];	// modification 저장할 배열 
	int mod_cnt=0;			// modification 개수
	fprintf(objfile,"H%s\t%06X%06X\n",start_symbol,start_address,end_address-start_address);	// obj file 첫 줄 출력
	bits bit;	// nixbpe 비트 구조체
	while(1){
		
		memset(line,'\0',sizeof(line));				// 배열들 초기화
		if(fgets(line,sizeof(line),tempfile)==NULL) break;
		bit_init(&bit);
		memset(loc_cnt,'\0',sizeof(loc_cnt));
		memset(symbol,'\0',sizeof(symbol));
		memset(opcode,'\0',sizeof(opcode));
		memset(operand1,'\0',sizeof(operand1));
		memset(operand2,'\0',sizeof(operand2));
		for(int i=0;i<strlen(line);i++){	// 개행문자 삭제
			if(line[i] == '\n') line[i] = '\0';
		}
		cnt = sscanf(line,"%d %s %s %s %s %s",&line_num,loc_cnt,symbol,opcode,operand1,operand2);	// line 쪼개기
		for(int q=0;q<strlen(line);q++){
			if(line[q]==',' && cnt == 5){	// operand가 2개면서 토큰이 5개인 경우 symbol 제거
				strcpy(operand2,operand1);
				strcpy(operand1,opcode);
				strcpy(opcode,symbol);
				symbol[0] ='\0';
			}
			if(line[q]==','){	// , 문자 삭제
				operand1[strlen(operand1)-1] = '\0';
			}
			if(line[q]=='?'){	// 앞서 base, end 부분 ? 삭제 (symbol token을 세기 위함)
				line[q] = ' ';
			}
		}
		for(int i = strlen(line);i<=40;i++) line[i] = ' ';	// 출력 형식 맞추기위해 공백 추가 
		if(cnt == 4){	// 토큰 개수에 따라 각각 변수에 올바른 값 저장
			strcpy(operand1,opcode);
			strcpy(opcode,symbol);
			symbol[0]='\0';
		}
		if(cnt == 3){
			strcpy(opcode,symbol);
			symbol[0]='\0';
		}
		pc = loc_arr[line_iter++];	// loc_arr를 통해 현재 pc값 불러옴
		if(loc_cnt[0] == '.'){	// 주석일 경우
			fprintf(lstfile,"%s\n",line);
			continue;
		}
		if(!strcmp(opcode,"START")){	// start인 경우
			fprintf(lstfile,"%s\n",line);
			continue;
		}
		else if(!strcmp(opcode,"END")){	// end인 겨우
			fprintf(lstfile,"%s\n",line);
			break;
		}
		else if(!strcmp(opcode,"BASE")){	//base인 경우
			base = find_symbol(operand1);	//base register 저장
			fprintf(lstfile,"%s\n",line);
			continue;
		}
		if(operand1[0] == '#' || operand1[0] == '@'){ // simple인 경우가 default로 저장되어 있음
			if(operand1[0] == '#') bit.n=0;	// immediate
			else bit.i = 0; // indirect
			for(i=0;i<strlen(operand1)-1;i++) operand1[i] = operand1[i+1];	// #,@ 제거
			operand1[i]='\0';
		}
		if(opcode[0] == '+'){
			for(i=0;i<strlen(opcode)-1;i++) opcode[i] = opcode[i+1];	// + 제거
			opcode[i] = '\0';
			bit.p = 0;	//4형식 처리
			bit.e = 1;
			type =4;
			op_number = hash_search(opcode,0);	// opcode number 불러오기
		}
		else{
			op_number = hash_search(opcode,0);	// 형식 저장
			if(op_number!=-1){
				if(!strcmp(format,"1")) type =1;
				else if(!strcmp(format,"2")) type =2;
				else type =3;
			}
			else{	// opcode를 못 찾았을 때
				if(!strcmp(opcode,"BYTE") || !strcmp(opcode,"WORD")){	// byte, word인 경우
					fprintf(obj_temp,"%s ",loc_cnt);
					fprintf(lstfile,"%s\t",line);
					if(operand1[0] == 'X'){		// x 형식
						for(i=2;i<strlen(operand1)-1;i++){	// x내의 수 입력
							fprintf(lstfile,"%c",operand1[i]);
							fprintf(obj_temp,"%c",operand1[i]);
						}
					}
					else if(operand1[0] == 'C'){	// c형식
						for(i=2;i<strlen(operand1)-1;i++){
							int tt = operand1[i];
							fprintf(lstfile,"%02X",tt);	// c 내의 char 입력
							fprintf(obj_temp,"%02X",tt);
						}
					}
					else{	// 제3의 형식 에러 처리
						printf("input error, line: %d\n",line_num/5);
						return 0;
					}
					fprintf(lstfile,"\n");
					fprintf(obj_temp,"\n");
				}
				else if(!strcmp(opcode,"RESB") || !strcmp(opcode,"RESW")){	// resb, resw인 경우
					fprintf(lstfile,"%s\n",line);
					fprintf(obj_temp,"1 1\n");	// 이후 obj file를 위해 1 1 출력
				}
				else{	// 제3의 형식인 경우
					printf("input error, line:%d\n",line_num/5);
					return 0;
				}
				continue;
			}
		}
		if(type == 1){	// 1형식 그대로 출력
			fprintf(lstfile,"%s\t%02X\n",line,op_number);
			fprintf(obj_temp,"%s %02X\n",loc_cnt,op_number);
		}
		else if(type == 2){	// 2형식
			int reg1=0,reg2=0;	
			for(int i=0;i<10;i++){	// operand1, 2 의 register 찾기, 못 찾은 경우 0
				if(i==7) continue;
				if(!strcmp(operand1,reg[i])) reg1 = i;
				if(!strcmp(operand2,reg[i])) reg2 = i;
			}
			fprintf(lstfile,"%s\t%02X%X%X\n",line,op_number,reg1,reg2);
			fprintf(obj_temp,"%s %02X%X%X\n",loc_cnt,op_number,reg1,reg2);
		}
		else if(type == 3 || type == 4){	// 3, 4형식
			int symbol_number = find_symbol(operand1); // symbole table 검색
			int sub = symbol_number-pc;	// pc relative default
			if(type == 4){	// 4형식인 경우
				bit.p = 0; bit.e=1;
				sub  = symbol_number;
				if(bit.n*2+bit.i==3){	// simple인 경우 modification 저장
					int dummyflag=0;
					mod_address[mod_cnt] = hex_check(loc_cnt,&dummyflag)+1;
					mod_cnt++;
				}
			}
			if(symbol_number==0){	// 상수입력인 경우
				sub = atoi(operand1);	// 상수 저장
				bit.p=0;
			}
			if(!strcmp(operand2,"X")) bit.x=1;	// x bit set
			if(type==3 && (sub < -2048 || sub>2047)){	// pc relative 범위 벗어난 경우 base relative로 전환
				bit.p =0; bit.b=1;
				sub = symbol_number - base;
			}
			int xbpe = bit.x*8+bit.b*4+bit.p*2+bit.e; // xbpe bit 계산
			op_number+=bit.n*2+bit.i;	// ni bit 포함하여 opcode number 저장
				
			if(type == 3){
				if(sub<0) sub = 4096+sub;	// 음수 2's complement 처리
				fprintf(lstfile,"%s\t%02X%X%03X\n",line,op_number,xbpe,sub);
				fprintf(obj_temp,"%s %02X%X%03X\n",loc_cnt,op_number,xbpe,sub);
			}
			else{
				if(sub<0) sub = 1048576+sub; 
				fprintf(lstfile,"%s\t%02X%X%05X\n",line,op_number,xbpe,sub);
				fprintf(obj_temp,"%s %02X%X%05X\n",loc_cnt,op_number,xbpe,sub);
			}
		}
	}
	fclose(obj_temp);
	fclose(tempfile);
	
	FILE *obj_read = fopen("obj_temp","r");	// 임시파일 읽어오기 locctr, object code가 저장되어 있음
	if(obj_read == NULL){
		printf("file open error\n");
		return 0;
	}
	char *locctr;
	char *obj_code;
	char obj_line[100];
	cnt=0;
	int endl=1;	// 개행이 연속되지 않게 하기 위한 변수
	int start = -1;	// 각 줄의 start 주소 저장
	int first=-1;	// E출력시 필요한 start 첫 주소 저장
	int hex_flag=0;
	memset(obj_line,'\0',sizeof(obj_line));
	while(1){
		memset(line,'\0',sizeof(line));
		if(fgets(line,sizeof(line),obj_read)==NULL) break;
		locctr = strtok(line," ");	// locctr, object code로 쪼갬
		obj_code = strtok(NULL,"\n");
		if(!strcmp(locctr,"1") || cnt+strlen(obj_code)/2>30){	// resb, resw인 경우, 한 줄 크기를 넘기는 경우
			if(endl==1) continue;	// 연속 개행 처리
			fprintf(objfile,"T%06X%02X%s\n",start,cnt,obj_line);	// object file에 출력 
			memset(obj_line,'\0',100);	// 초기화
			start = -1;
			cnt=0;
			if(strcmp(locctr,"1")){	// 크기를 넘겨 개행한 경우 이번 입력 저장
				cnt += strlen(obj_code)/2;
				start = hex_check(locctr,&hex_flag);
				strcat(obj_line,obj_code);
			}
			endl=1;	// 개행 처리
		}
		else{
			endl=0;	// 개행 처리
			strcat(obj_line,obj_code);	// 각 줄에 입력된 object code 덧붙이기
			cnt+=strlen(obj_code)/2;	// object code 길이 저장
			if(start == -1) start = hex_check(locctr,&hex_flag);	// 시작인 경우
			if(first == -1) first = start;	// 첫 시작인 경우
		}
	}
	if(endl == 0){	// 아직 출력 덜한게 있는 경우
		fprintf(objfile,"T%06X%02X%s\n",start,cnt,obj_line);
	}
	for(int i=0;i<mod_cnt;i++){	// modification 출력
		fprintf(objfile,"M%06X05\n",mod_address[i]);
	}
	fprintf(objfile,"E%06X\n",first);	// e 출력
	printf("[%s], [%s]\n",lst,obj);	//assemble 명령 출력
	fclose(objfile);
	fclose(lstfile);
	return 1;
}
int find_symbol(char *symbol){	// symbol table에서 symbol을 찾는 함수 찾은 경우 symbol address return
	for(int i=0;i<sym_cnt;i++){
		if(!strcmp(symbol_table[i],symbol)){
			return symbol_address[i];
		}
	}
	return 0;
}
void bit_init(bits *bit){	// nixbpe bit 초기화, pc relative, simple default
	bit->i = 1; bit->n= 1;
	bit->p =1; bit->b = 0;
	bit->x =0; bit->e = 0;
}
int hash_function(char mnemonic[10]){	// ascii code를 이용한 hash function
	int cnt=0;
	for(int i=0;i<strlen(mnemonic);i++){
		cnt+=mnemonic[i];
	}
	return cnt%20;
}
void hash_push(int opcode, char mnemonic[10], char form[10]){
	int index = hash_function(mnemonic); // hash[index] 참조하여 link 끝에 저장
	hashNode* hashTemp = malloc(sizeof(hashNode));
	hashTemp = hashTable[index];
	hashNode* node = malloc(sizeof(hashNode));
    while(hashTemp->next!=NULL){
	    hashTemp = hashTemp->next;
	}
	strcpy(node->mnemonic,mnemonic);
	node->opcode = opcode;
	strcpy(node->form,form);
	hashTemp->next = node;
}
void hash_print(){		//hash table 출력
	for(int i=0;i<20;i++){	
		hashNode* node = malloc(sizeof(hashNode));
		node = hashTable[i];
		printf("%d : ",i);
		while(node->next!=NULL){
			node = node->next;
			printf("[%s,%x]",node->mnemonic,node->opcode);
			if(node->next!=NULL) printf(" -> ");
		}
		printf("\n");
	}
}
int hash_search(char mnemonic[10], int print_flag){		// key 검색
	int index = hash_function(mnemonic);
	int flag=-1;
	hashNode *node = malloc(sizeof(hashNode));
	node = hashTable[index];
	do{
		node = node->next;
		if(!strcmp(mnemonic,node->mnemonic)){	//찾은경우 출력
			if(print_flag == 1) printf("opcode is %02X\n",node->opcode);
			flag=node->opcode;	// 찾음을 의미
			strcpy(format, node->form);
			break;
		}
	}while(node->next!=NULL);
	if(flag == -1 && print_flag == 1) printf("The mnemonic could not be found\n");
	return flag;
}
void print_dump(int start, int end){		//dump 출력
	if(end>MAX_BUFF) end = MAX_BUFF;
	int t = start%16;
	int row=0,i=0,cnt=0,cnt1=0,m=end-start;
	if(t) t=1;
	do{
		row = start/16+i;
		printf("%05X ",row*16);	// 형식에 맞게 row값 조정
		for(int j=0;j<16;j++){
			if((i==0 && j<start%16) || m<cnt) printf("   ");
			else{
				cnt++;
				printf("%02X ",memory[row][j]);
			}
		}
		printf("; ");
		for(int j=0;j<16;j++){
			if((i==0 && j<start%16) || m<cnt1) printf(".");
			else{
				cnt1++;
				if(memory[row][j]>=32 && memory[row][j]<=126) printf("%c",memory[row][j]);
				else printf(".");
			}
		}
		printf("\n");
		i++;
	}while(m>=cnt);
}
int hex_check(char* token1, int *errorFlag){	// 16진수 이외의 입력 예외 처리
	int address=0;
	for(int i=0;i<strlen(token1);i++){
		if(token1[i] >='0' && token1[i]<='9'){
			address+=token1[i]-'0';
		}
		else if(token1[i] >= 'A' && token1[i] <= 'F'){
		    address+=token1[i] -'A' + 10;
		}
		else if(token1[i] >= 'a' && token1[i] <= 'f'){
			address+=token1[i] - 'a' + 10;
		}
		else{
		    *errorFlag=1;	// errorFlag = 1 이면 에러
			break;
		}
		address*=16;
	}
	address/=16;
	if(address>MAX_BUFF) *errorFlag=1;
	return address; 
}
char* nomalization(char command[100]){	// 공백, 탭 등 예외 처리
	char *temp = malloc(sizeof(char)*100);
	memset(temp,'\0',100);
	char t;
	int i,tempcnt=0;
	int flag=0;
	for(i=0;i<strlen(command);i++){
		t = command[i];
		if(t == '\t' || t=='\n') continue; // tab이 들어온 경우
		if(t == ' ' && flag == 0) continue;	// 첫 시작이 공백일 경우
		if(t == ' ' && command[i+1] == ' ') continue;	// 공백이 연속된 경우
		if(t == ' ' && (command[i+1] == '\n' || i==strlen(command)-1)) continue; // 마지막이 공백일 경우
		flag=1;
		temp[tempcnt] = t;
		tempcnt++;
	}
	return temp;
}
void node_init(historyNode *node){
    node->commandCnt=0;
    node->next=NULL;
}
historyNode* command_push(char command[100], historyNode *tail){	// history를 위해 command 입력시 linked list에 저장
    historyNode *node = malloc(sizeof(historyNode));
    node_init(node);
    node->commandCnt = tail->commandCnt+1;	// 꼬리 참조
    strcpy(node->commandName,command);
    tail->next = node;
    return node;	// 새로운 node return하여 꼬리 갱신
}
void print_history(historyNode *node){		//history 출력
    while(node->next!=NULL){
		node = node->next;
        printf("%d\t%s\n",node->commandCnt,node->commandName);
    }
}
void print_help(){	//help 출력
    printf("h[elp]\nd[ir]\nq[uit]\nhi[story]\ndu[mp] [start, end]\n");
    printf("e[dit] address, value\nf[ill] start, end, value\n");
    printf("reset\nopcode mnemonic\nopcodelist\n");
	printf("assemble filename\ntype filename\nsymbol\n");
}
void print_dir(){	//dir 출력
    DIR *dirStatus = opendir(".");
    struct dirent *dir;
    struct stat buf;
    int cnt=0;
    if(dirStatus == NULL){		// 에러 처리
        printf("Directory error\n");
        return;
    }
    for(dir = readdir(dirStatus); dir != NULL; dir = readdir(dirStatus)){	
        lstat(dir->d_name, &buf);
		if(!strcmp(dir->d_name,".") || !strcmp(dir->d_name,"..")) continue;	// . 과 ..은 삭제
		cnt++;
        if(S_ISDIR(buf.st_mode))   // 해당 파일이 dir일 경우
            printf("%18s/",dir->d_name);
        else if(S_IXGRP & buf.st_mode)	// 해당 파일에 대해 실행권한이 있을 경우 -> 실행파일인 경우
            printf("%18s*",dir->d_name);
        else
            printf("%18s",dir->d_name);	// 나머지 파일
        if(cnt==4){	//4개 출력마다 개행
            cnt=0;
            printf("\n");
        }
    }
    printf("\n");
    closedir(dirStatus);
}

