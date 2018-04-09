#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>

#define MAX 4096
#define MIN 256
#define SED_LENGTH 7

int same = 0;
char arg[11][15];
char opStr[6][MAX], Ppath[MIN];
int ob=0, ou=0, oi=0, oe=0, od=0, op=0, os=0, oP=0, oes=0, ois=0; // option number.
int oui, oii, oei, odi, oPi, oesi, oisi; // option index number.
int icheck=0, echeck=1, ischeck=0, escheck=1, depth=0, DEPTH;
/*
전역변수 : 옵션에 관련된 값들(ob=option -b, obi=option -b index)을 주로 선언했으며 
same은 치환여부를 결정하기위해 사용하고 icheck등은 -i, -e, -es, -is 옵션의 성공여부를 결정한다.
*/

void ssu_sed(int count, char parsing[10][MAX]);
void m_strcpy(char *dest, char *src);
void p_strcat(char *dest_str, char *src_str);
void re_opendir(char *path, char *src_str, char *dest_str);
void process_in_file(char *path, char *src_str, char *dest_str);
int m_strlen(char *str);
int m_parsing(char (*parsing)[MAX], char *command, int count);
void m_replace(char *file_str, char *src_str, char *dest_str, char *result);
void m_strcat(char *dest_str, char *src_str);
void init();
int check_ie_option(char file_str[MAX], int index);
void m_itoa(char *buf, int num);
int m_strcmpi(char str1[MAX], char str2[MAX]);

/*
string관련 구현함수들과 ssu_sed관련 디렉터리, 파일, 치환함수를 구현
*/

int m_strcmp(char *a, char *b)
{
	/*strcmp함수를 구현. 두번째 스트링의 끝까지 비교하면서 
	다른부분이 나오면 0을 리턴하고
	끝까지 같을 시 1을 리턴한다.
	*/
	for(int i=0; b[i] != '\0'; i++){
		if( a[i] != b[i] ){
			return 0;
		}
	}
	return 1;
}

int main(){
	struct timeval timeBefore, timeAfter;
	int i=0, j=0, l, count=0, start, end, length;
	char command[MAX], viStore[30], parsing[10][MAX];
	char sedStr[SED_LENGTH] = "ssu_sed";
/*
메인함수에서는 기본적인 system() 호출을 사용하여 ssu_shell의 기본 기능을 구현하고
명령어들이 수행되는 시간을 gettimeofday()를 사용하여 계산한다.
또한 입력되는 인자들을 parsing하여 그 인자들과 길이를 ssu_sed()에 넘겨준다.
추가로 ssu_shell안에서 한번 vi명령어를 실행하면 다음부터는 !v로 전에 실행했던 vi를 재실행할수있게했다.
*/
	for(i=0; i<10; i++)
		sprintf(arg[i], "arg%d : ", i);

	while(1){

		count = 0;

		i=0, j=0;

		printf("20132437 $ ");
		fgets(command, MAX, stdin);

		if(command[0] == 'v' && command[1] == 'i'){
			for(i=0; command[i]!='\n'; i++){
				viStore[i] = command[i];
			}
			viStore[i] = '\0';
		}

		// 인자로 넘겨 받은 스트링을 command에 저장하고 그 길이를 구한 후 parsing한다.
		while(command[i]!='\n')
			i++;

		length = i;

		/* parsing할 때 공백을 모두 제거하여 공백이 잘못 입력되어도 실행 가능하게 구현하였다.
		예를들어 ls -l 명령어를 실행할 때 l s - l은 입력하지 못하지만 ls     -l은 입력가능하다.
		*/
		for(i=0; command[i]!='\n'; i++){
			if( command[i] != ' ' ){
				break;
			}
		}
		start=i;

		// 그냥 엔터가 입력될 시에 while문의 처음으로 돌아가 다시 실행된다.
		if(command[start] == '\n')
			continue;

		/* 띄어쓰기 단위로 끊어진 인자의 개수를 구한다. 이때 띄어쓰기가 여러번 입력되어도 
		한번만 인식하도록 하였다.
		*/
		for(i=1; i<MAX; i++){
			if( (command[i] == ' ') && (command[i-1] != ' ') ){
				count++;
			}
			else if( (command[i] == '\n') && (command[i-1] != ' ') ){
				count++;
				break;
			}
			else if(command[i] == '\n')
				break;
		}


		command[i] = '\0';

		// command스트링을 parsing하여 parsing스트링에 저장하고 몇개의 인자가 있는지 j에 저장한다.
		j = m_parsing(parsing, command, count);

		//	만약 인자의 개수가 10개가 넘으면 처음으로 돌아간다.
		if(j > 10){
			printf("No more than 10!\n");
			continue;
		}
		
		/* gettimeofday()함수를 이용해 명령어의 실행 시간을 구하고 그 값을 마이크로세컨드 단위로 구한다.
		또한 ssu_sed()함수가 호출될 때와 !v가 호출될 때, 그렇지 않고 system()함수를 호출 할 때를 지정한다.
		*/
		if(gettimeofday(&timeBefore, NULL) == -1)
			perror("Time Error");

		if( m_strcmp(parsing[0], sedStr) ){
			ssu_sed(j, parsing);
			init();
		} 

		else if(command[0] != '!'){
			system(command);
		}
		else {
			if(command[1] == 'v')
				system(viStore); 
		}

		if(gettimeofday(&timeAfter, NULL) == -1)
			perror("Time Error");

		printf("time : %.3f\n",(timeAfter.tv_usec - timeBefore.tv_usec)/1000000.0);
	}
	return 0;

}

void ssu_sed(int count, char parsing[10][MAX]){
	/* ssu_sed()함수에서는 기본적으로 인자를 가지고 사용할 변수에 저장하고, 옵션을 받으며 
	다른 여러 함수를 호출하여 ssu_sed()의 기능을 구현한다.
	*/
	char dest_str[15], src_str[15], option[10][15];
	int fd;
	DIR *dp;
	struct stat st;
	struct dirent *d;
	char path[MAX], rootPath[MAX];

	if(count < 3){
		// 인자의 개수가 부족할 경우 사용법을 출력하고 종료한다.
		fputs("Usage : ssu_sed [TARGET] [SRC_STR] [DEST_STR] [OPTION]\n", stdout);
		return;
	}
	
	// 파일을 다루는데 계속 사용할 path변수를 초기화 한다.
	path[0] = 0;

	// arg%d : %s형식을 printf없이 구현하였다. arg%d는 미리 저장해둔 값을 사용하고
	// %s는 parsing된 값을 사용한다.
	for(int i=0; i<count; i++){
		write(1, arg[i], m_strlen(arg[i]));
		puts(parsing[i]);
	}

	/* 옵션이 체크되는 부분이다. 미리 구현해둔 m_strcmp함수를 사용하여 옵션을 구분하고
	중복 검사를 실시한다. 중복이 검사되면 경고 출력과 함께 종료되어 명령어를 다시 입력받는다.
	<STRING>을 받는 옵션의 경우 opStr배열에 그 값만 따로 저장하고 인덱스로 미리 지정해둔 변수에 저장하여 
	사용한다.
	*/
	int l=0;
	for(int i=4; i<count; i++) {
		if( m_strcmp(parsing[i], "-b") ) {
			if( ob ){
				puts("Do not REPETITION\n");
				return;
			}
			ob = 1;
		}
		else if( m_strcmp(parsing[i], "-u") ) {
			if( ou ){
				puts("Do not REPETITION\n");
				return;
			}
			ou = 1;
			oui = l;
			m_strcpy(opStr[l++], parsing[++i]);
		}
		else if( m_strcmp(parsing[i], "-es") ) {
			if( oes || oe){
				puts("Do not REPETITION\n");
				return;
			}
			oes = 1;
			oesi = l;
			m_strcpy(opStr[l++], parsing[++i]);
		}
		else if( m_strcmp(parsing[i], "-is") ) {
			if( ois || oi){
				puts("Do not REPETITION\n");
				return;
			}
			ois = 1;
			oisi = l;
			m_strcpy(opStr[l++], parsing[++i]);
		}
		else if( m_strcmp(parsing[i], "-i") ) {
			if( oi ){
				puts("Do not REPETITION\n");
				return;
			}
			oi = 1;
			oii = l;
			m_strcpy(opStr[l++], parsing[++i]);
		}
		else if( m_strcmp(parsing[i], "-e") ) {
			if( oe ){
				puts("Do not REPETITION\n");
				return;
			}
			oe = 1;
			oei = l;
			m_strcpy(opStr[l++], parsing[++i]);
		}
		else if( m_strcmp(parsing[i], "-d") ) {
			if( od ){
				puts("Do not REPETITION\n");
				return;
			}
			od = 1;
			odi = l;
			m_strcpy(opStr[l++], parsing[++i]);
			DEPTH = atoi(parsing[i]);
		}
		else if( m_strcmp(parsing[i], "-p") ) {
			if( op ){
				puts("Do not REPETITION\n");
				return;
			}
			op = 1;
		}
		else if( m_strcmp(parsing[i], "-s") ) {
			if( os ){
				puts("Do not REPETITION\n");
				return;
			}
			os = 1;
		}
		else if( m_strcmp(parsing[i], "-P") ) {
			if( oP ){
				puts("Do not REPETITION\n");
				return;
			}
			oP = 1;
			oPi = l;
			m_strcpy(opStr[l++], parsing[++i]);
			mkdir(parsing[i], 0755);
		}
		else{
			puts("Options : -b, -u, -i, -e, -d, -p, -s, -P");
			return;
		}
	}

	// realpath()함수를 통해 현재 TARGET 디렉터리의 루트를 얻는다.
	if (realpath(parsing[1], rootPath) == NULL) {
		perror("realpath error");
		return;
	}

	// -P옵션이 설정될 시 Ppath배열에 path와 TARGET만 다른 경로를 저장함으로써 새로운 경로를 개척한다.
	if(oP)
		realpath(opStr[oPi], Ppath);

	// path에 TARGET의 루트 경로를 복사하고 src_str에는 세번째 인자, dest_str에는 네번째 인자를 복사한다.
	p_strcat(path, rootPath);
	m_strcpy(src_str, parsing[2]);
	m_strcpy(dest_str, parsing[3]);

	// TARGET디렉터리부터 open하여 탐색을 시작한다.
	re_opendir(path, src_str, dest_str);
	
	// 탐색을 마친 후 디렉터리를 닫고 종료한다.
	closedir(dp);
	return;
}

void re_opendir(char *path, char *src_str, char *dest_str)
{
	// 디렉터리를 Recursion을 이용하여 탐색한다. 
	DIR *dp;
	struct stat st;
	struct dirent *d;
	int len=0, Plen=0;

	// 현재까지의 path의 길이를 구한다. 나중에 다른 경로를 탐색할 때 이전 path를 삭제하기 위해 사용한다.
	len = m_strlen(path);

	if ((dp = opendir(path)) == NULL){
		perror(path);
		return;
	}

	while( (d = readdir(dp)) != NULL) {
		/* stat구조체와 dirent구조체를 이용하여 디렉터리를 탐색하여 파일혹은 디렉터리를 읽고 
		파일일 시 process_in_file(), 디렉터리일 시 재귀호출을 시행한다.
		파일 탐색 시 현재디렉터리와 부모디렉터리에 대한 경로는 탐색하지 않는다.
		*/
		if( m_strcmp(d->d_name, ".") || m_strcmp(d->d_name, "..") )
			continue;
		// 현재 path에 탐색한 파일 혹은 디렉터리를 '/'를 붙여 경로를 만든다.
		p_strcat(path, d->d_name);

		if(oP){
			// 옵션 -P가 설정되면 path가 했던 같은 작업을 Ppath에도 수행하여 경로를 저장한다.
			Plen = m_strlen(Ppath);
			p_strcat(Ppath, d->d_name);
		}
		
		/* stat구조체의 매크로함수를 이용하여 새로 추가된 경로가 디렉터리인지 파일인지 검사한다.
		옵션 -d를 위해서 파일 혹은 디렉터리 처리 함수를 호출 할때 depth가 1씩 올라간다.
		*/
		if(lstat(path, &st) < 0)
			perror(path);
		else{
			if(S_ISREG(st.st_mode)){
				depth++;
				process_in_file(path, src_str, dest_str);
				depth--;
			}
			else if(S_ISDIR(st.st_mode)){
				/* 하위 파일이 디렉터리일 시 re_opendir()함수를 재귀호출한다. 이 때 -P옵션 설정 시 
				Ppath에 맞는 디렉터리를 생성하고 끝나고 리턴되었을 때 그 디렉터리를 삭제한다.
				mkdir은 디렉터리가 이미 있거나 다른이유로 만들 수 없으면 만들지 않고 
				rmdir은 디렉터리안에 다른 파일이 존재하면 지우지 않는다.
				*/
				depth++;
				if(oP)
					mkdir(Ppath, 0755);
				re_opendir(path, src_str, dest_str);
				if(oP)
					rmdir(Ppath);
				depth--;
			}
		}

		// path와 Ppath를 아까 구한 길이를 제외하고 초기화시켜 다음 파일의 경로를 탐색한다.
		for(int i=len; path[i] != '\0'; i++){
			path[i] = '\0';
		}
		for(int i=Plen; Ppath[i] != '\0'; i++) {
			Ppath[i] = '\0';
		}
	}
	closedir(dp);
	return;
}

void process_in_file(char *path, char *src_str, char *dest_str)
{
	// 옵션 -d설정 시 지정한 DEPTH 이상으로 못들어가게 한다.
	if(od && (depth>DEPTH))
		return;

/* process_in_file()함수는 파일을 처리하는 함수이다. 파일의 문자를 저장하고
치환하여 조건에 따라 다시 덮어쓰는 작업을 한다.
*/
	char parsing[100][MAX];
	char file_str[MAX];
	char result[MAX], name[MAX];
	FILE *fp, *tmp_fp, *P_fp;
	int count=0;
	int len = m_strlen(src_str);
	int success = 0, change=0;
	int line=0, line_same[1024];

// 옵션에 따른 입력 조건을 나타내는 변수를 초기화한다.
	icheck = 0;
	echeck = 1;
	ischeck = 0;
	escheck = 1;

/* 지정한 path에 해당하는 파일을 읽기 쓰기로 연다. 이때 임시로 치환된 문자열을 저장할 파일을 
tmpfile()호출을 사용하여 연다.
*/
	if( (fp=fopen(path, "rw")) == NULL) {
		perror(path);
		return;
	}
//임시파일.
	if ((tmp_fp = tmpfile()) == NULL) {
		return;
	}

	while(fgets(file_str, MAX, fp) != NULL) {
		/* fp가리키는 파일에서 문자열을 한 줄씩 가져와 file_str에 저장하고 m_replace()함수를 통해
		치환할 문자열이 있는지 검사하고 치환한 후 수행 여부를 판단한다.
		line변수는 -p옵션 설정시 줄번호 출력을 위해 사용한다.
		*/
		line++;
		m_replace(file_str, src_str, dest_str, result);
		if(same == 1){
			// m_replace()에서 치환 과정중 치환이 한번이라도 발생하면 success를 1로만들어 파일 치환을 한다.
			success = 1;
			line_same[count++] = line;
		}
		// 치환 결과인 result배열을 임시파일에 쓴다.
		fputs(result, tmp_fp);
	}

//조건:치환이 일어났으며, 옵션 i,e가 설정되지 않았거나 설정됐다면 치환할 조건을 만족시킬 때.
	if(success && ((oi==0 && oe==0) || (oi && oe==0 && icheck) || (oe && oi==0 && echeck) || (oe && oi && icheck && echeck))){

		if((ois && ischeck==0) || (oes && escheck==0)){
			// 옵션 -is와 -es가 설정되었을 시 치환조건을 만족하지 않으면 
			// change를 0으로 만들고 치환하지않는다.
			change = 0;
		}

		else {
			/* 파일간의 치환이 일어난다. fp가 가리키던 파일을 닫고 "w"모드로 다시 열어
			파일이 비워지게 한후, 임시파일에 있던 문자열을 그 파일에 한줄씩 적는다.
			*/
			change=1;
			char buf[MAX];
			fseek(tmp_fp, 0, SEEK_SET);

			if(oP==0){
				fclose(fp);
				if (( fp = fopen(path, "w")) == NULL){
					perror(path);
					return;
				}

				while(fgets(buf, MAX, tmp_fp) != NULL) {
					fputs(buf, fp);
				}
			} else {
				/* 만약 옵션 -P가 설정되었다면 fp가 가리키던 파일은 수정하지 않고 냅두며 
				Ppath에 해당하는 경로에 파일을 열거나 생성하고 그 곳에 임시파일에 있던
				내용을 적는다.
				*/
				if( (P_fp = fopen(Ppath, "w")) == NULL ) {
					perror(Ppath);
				}
				while(fgets(buf, MAX, tmp_fp) != NULL) {
					fputs(buf, P_fp);
				}
				fclose(P_fp);
			}
		}
	}

	// 파일의 치환 여부에 따라 path의 끝에 failed or success를 붙인다.
	if(change==0)
		m_strcat(path, " : failed");
	else{
		if(op) {
			// 옵션 -p설정 시 path와 아까 저장했던 line번호를 출력한다.
			// If option -p.
			char num_buf[1024];
			for(int i=0; i<count; i++) {
				m_itoa(num_buf, line_same[i]);
				write(1, path, m_strlen(path));
				write(1, " : ", 3);
				puts(num_buf);
			}

		}

		m_strcat(path, " : success");
	}

	puts(path);
	fclose(fp);
}

void m_replace(char *file_str, char *src_str, char *dest_str, char *result)
{
	/* 파일의 문자열 치환을 구현한 함수이다. 옵션 -u가 설정되었을 때 파일의
	소유자를 알아내는 기능도 구현하였다. 문자열 치환은 우선 치환할 문자 SRC_STR이
	file_str에 있는지 temp배열을 이용해 검사하고 있으면 temp1, temp2 배열에 치환할 문장의 각각 앞뒤 문장을
	저장한뒤 result배열에 순서대로 이어 붙인다. 이때 미리 구현한 m_strcat()함수를 사용한다.
	*/
	int i=0, j=0, l=0, p;
	char temp[MAX], temp1[MAX], temp2[MAX];
	int srclen = m_strlen(src_str);

	same = 0;

	for(i=0; result[i]!='\0'; i++)
		result[i] = 0;
	i=0;
	// If option -u.
	struct passwd *pw;
	pw = getpwuid(geteuid());
	if (pw && ou) {
		if(m_strcmp(pw->pw_name, opStr[oui]) == 0){
			m_strcpy(result, file_str);
			return;
		}
	}

	while(file_str[i] != '\0'){
		//조건:file에서 가져온 문자열에 src_str의 첫글자가 있는지 검사한다. -s옵션 시 그 문자의
		//대소문자도 검사한다.
		if((file_str[i] == src_str[0]) || (os&&( (file_str[i]+32==src_str[0]) || (file_str[i]-32 == src_str[0]) ))){
			p=i;

			if( ob == 1){
				// 옵션 -b가 설정되어 있으면 공백을 제거하여 temp배열에 넣는다.
				for(j=0; j<srclen; j++){
					if((file_str[p] != ' ') || (src_str[j] == ' ')){
						temp[j] = file_str[p];
						p++;
					}
					else {
						j--;
						p++;
					}
				}
				temp[j] = '\0';
			}
			else {
				// src_str의 첫문자가 나온 부분부터 src_str의 길이만큼 temp배열에 저장하여
				// m_strcpy()함수로 비교한다.
				for(j=0; j<srclen; j++){
					temp[j] = file_str[p];
					p++;
				}
				temp[j] = '\0';
			}

			if( m_strcmp(temp, src_str) || (os&&m_strcmpi(temp, src_str))){
				// temp와 src_str이 같으면 file_str의 나머지 두 부분도 짤라서 result에 붙인다.
				same=1;
				for(j=0; j<i; j++) {
					temp1[j] = file_str[j];
				}
				temp1[j] = '\0';
				for(j=p; file_str[j] != '\0'; j++) {
					temp2[l] = file_str[j];
					l++;
				}
				temp2[l] = '\0';
				m_strcat(result, temp1);
				m_strcat(result, dest_str);
				m_strcat(result, temp2);
			}
		}
		i++;
	}

// 문자의 치환 여부, 옵션에 따른 치환 여부를 검사하여 각각의 변수에 저장하고 리턴한다.
	if(same==0)
		m_strcpy(result, file_str);

	if(oi && (check_ie_option(file_str, oii)==1)) {
		// -i옵션을 체크한다.
		icheck = 1;
	}  
	if(ois && (check_ie_option(file_str, oisi) == 1)) {
		// -is옵션을 체크한다.
		ischeck = 1;	
	}

	if(oe && (check_ie_option(file_str, oei)==1)) {
		// -e옵션을 체크한다.
		echeck = 0;
	} 
	if(oes && (check_ie_option(file_str, oesi) == 1)) {
		// -es옵션을 체크한다.
		escheck = 0;
	}

	return;
}

int check_ie_option(char file_str[MAX], int index)
{
	/* -i, -e, -is, -es옵션에 대한 치환여부를 검사하는 함수이다. 
	-es, -is옵션 설정시 우선 비교대상들을 소문자화 시켜놓고 같은 문자열이 있는지 검사한다.
	*/
	int i=0, p, k;
	char temp[MAX];

	if(oes || ois) {
		for(i=0; file_str[i] != '\0'; i++) {
			if('A' <= file_str[i] && file_str[i] <= 'Z') {
				file_str[i] += 32;
			}
		}
		for(i=0; opStr[index][i] != '\0'; i++) {
			if('A' <= opStr[index][i] && opStr[index][i] <= 'Z') {
				opStr[index][i] += 32;
			}
		}
	}

	while(file_str[i] != '\0') {
		/* 검사할 두 대상의 첫문자가 같아지는 시점에 두 대상 중 한 대상만큼의 길이를
		떼어 검사하고 같으면 1 다르면 0을 리턴한다.
		이때 -es, -is옵션 시 m_strcmpi()함수로 대소문자 구분 없이 비교한다.
		*/
		if(file_str[i] == opStr[index][0]) {
			p = i;
			for(k=0; k<m_strlen(opStr[index]); k++) {
				temp[k] = file_str[p++];
			}
			temp[k] = 0;

			if( m_strcmp(temp, opStr[index]) )
				return 1;
			else if((oes || ois) && m_strcmpi(temp, opStr[index]) )
				return 1;
		}
		i++;
	}
	return 0;
}

void m_strcat(char *dest_str, char *src_str)
{
	// src_str을 dest_str에 이어붙인다.
	int i=0,j=0;

	while(dest_str[i] != '\0')
		i++;

	for(j=0; src_str[j]!='\0'; j++){
		dest_str[i] = src_str[j];
		i++;
	}

	dest_str[i] = '\0';

	return;
}

void p_strcat(char *dest_str, char *src_str)
{
	// m_strcat()과 비슷하지만 경로를 구할때 쓰기위해 두 문자열 사이에 '/'를 넣어준다.
	int i=0,j=0;

	while(dest_str[i] != '\0')
		i++;

	if(i!=0){
		dest_str[i] = '/';
		i++;
	}

	for(j=0; src_str[j]!='\0'; j++){
		dest_str[i] = src_str[j];
		i++;
	}
	dest_str[i] = '\0';

	return;
}

int m_strcmpi(char str1[MAX], char str2[MAX])
{
	/* 대소문자 구분없이 두 문자열을 비교하기 위해 사용한다.
	비교하기 전 두 문자열을 소문자화 시키고 비교하며 같으면 1 다르면 0을 반환한다.
	*/
	for(int i=0; str1[i] != '\0'; i++) {
		if( ('A' <= str1[i]) && ('Z' >= str1[i]) ) {
			str1[i] += 32;
		}
		if( ('A' <= str2[i]) && ('Z' >= str2[i]) ) {
			str2[i] += 32;
		}
	}

	for(int i=0; str2[i] != '\0'; i++){
		if( str1[i] != str2[i] ){
			return 0;
		}
	}
	return 1;
	// (if str1 = str2) return 1, (str1! = str2) return 0
}

void m_strcpy(char *dest, char *src)
{
	// src문자열을 dest배열에 복사한다. dest에 원래 있던 문자는 덮어씌어진다.
	int i;
	for(i=0; src[i] != '\0'; i++){
		dest[i] = src[i];
	}
	dest[i] = '\0';

	return;
}

int m_strlen(char *str)
{
	// 문자열의 길이를 구한다.
	int len=0;
	while (str[len] != '\0')
		len++;

	return len;
}

int m_parsing(char (*parsing)[MAX], char *command, int count)
{
	/* main()함수와 process_in_file()에서 사용한 parsing함수이다.
	이 함수는 띄어쓰기 단위로 문자열을 짤라 parsing배열에 저장한다.
	이때 띄어쓰기 2개이상은 1개로 친고 '\'입력시 띄어쓰기가 무시된다.
	*/
	int temp=0, i=0, j=0, l=0;
	for(j=0; j<count; j++){
		l=0;
		while(command[i] == ' '){
			i++;
		}
		while(command[i] != ' '){
			if(command[i] == '\0'){parsing[j][l]='\0'; break;}

			if((command[i] == '\\') &&(command[i+1] == ' ')){
				count--;
				while(command[++i] == ' ');

				parsing[j][l] = ' ';
				l++;
			}
			parsing[j][l] = command[i];
			i++;
			l++;
		}
		parsing[j][l] = '\0';
	}
	return j;
}

void init()
{
	// ssu_sed()함수가 끝난 후 호출하여 전역변수들을 초기화시켜 재사용 할 수 있게 한다.
	ob=0, ou=0, oi=0, oe=0, od=0, op=0, os=0, oP=0, oes=0, ois=0; // option number.
	oui=0, oii=0, oei=0, odi=0, oPi=0, oesi=0, oisi=0, DEPTH=0, depth=0; // option index number.
	for(int i=0; i<6; i++)
		opStr[i][0] = 0;
}

void m_itoa(char *buf, int num) 
{
	// num 숫자를 한자리씩 짤라 buf에 캐릭터형으로 저장한다.
	int i=0;
	while ( num != 0) {
		buf[i++] = (num%10+48);
		num = num/10;
	}
	buf[i] = 0;
}
