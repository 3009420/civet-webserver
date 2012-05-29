#include <WinSock2.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

char * HOST = "127.0.0.1";
unsigned short PORT = 80;
// char * RESOURCE = "/ajax/echo.cgi";
// char * RESOURCE = "/imagetest/00.png";
// char * RESOURCE = "/args.cgi";
// char * RESOURCE = "/_stat";
char * RESOURCE = "/_echo";

#define CLIENTCOUNT 20
#define TESTCYCLES 50


int sockvprintf(SOCKET soc, const char * fmt, va_list vl) {

  char buf[1024*8];
  int len = vsprintf_s(buf, sizeof(buf), fmt, vl);
  int ret = send(soc, buf, len, 0);
  return ret;
}


int sockprintf(SOCKET soc, const char * fmt, ...) {

  int ret = -1;
  va_list vl;
  va_start(vl, fmt);
  ret = sockvprintf(soc, fmt, vl);
  va_end(vl);
  return ret;
}


static struct sockaddr_in target = {0};
static CRITICAL_SECTION cs = {0};
static size_t expectedData = 0;
static DWORD availableCPUs = 1;
static DWORD totalCPUs = 1;
static unsigned good = 0;
static unsigned bad = 0;
unsigned long postSize = 0;


int WINAPI ClientMain(void * clientNo) {

  SOCKET soc;
  time_t lastData;
  size_t totalData = 0;
  int isBody = 0;
  int isTest = (clientNo == 0);
  int cpu = ((int)clientNo) % 1000;
  int timeOut = 10;

  if ((!isTest) && (((1<<cpu) & availableCPUs)!=0)) {
    SetThreadAffinityMask(GetCurrentThread(), 1<<cpu);
  }

  soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (soc==INVALID_SOCKET) {
    EnterCriticalSection(&cs);
    printf("\r\nClient %u cannot create socket\a\r\n", (int)clientNo);
    LeaveCriticalSection(&cs);
    return 1;
  }

  if (connect(soc, (SOCKADDR*)&target, sizeof(target))) {
    EnterCriticalSection(&cs);
    printf("\r\nClient %u cannot connect to server %s:%u\a\r\n", (int)clientNo, HOST, PORT);
    LeaveCriticalSection(&cs);
    return 2;
  }


  // Comment in just one of these test cases

  // "GET"
  // sockprintf(soc, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n", RESOURCE, HOST);

  // "GET" with 10000 bytes extra head data
  // sockprintf(soc, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n", RESOURCE, HOST);
  // {int i; for (i=0;i<(10000/25);i++) {sockprintf(soc, "Comment%04u: 1234567890\r\n");}}
  // sockprintf(soc, "\r\n");

  // "GET" with 2000 bytes of query string
  // sockprintf(soc, "GET %s?", RESOURCE);
  // {int i; for (i=0;i<200;i++) {sockprintf(soc, "1234567890");}}
  // sockprintf(soc, " HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n", HOST);

  // "POST <postSize> bytes"
  sockprintf(soc, "POST %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\nContent-Length: %u\r\n\r\n", RESOURCE, HOST, postSize);
  {unsigned long i; for (i=0;i<postSize/10;i++) {sockprintf(soc, "1234567890");} for (i=0;i<postSize%10;i++) {sockprintf(soc, ".");}}
  timeOut += postSize/10000;

  // "POST" with 2000 bytes of query string
  // sockprintf(soc, "POST %s?", RESOURCE);
  // {int i; for (i=0;i<200;i++) {sockprintf(soc, "1234567890");}}
  // sockprintf(soc, " HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\nContent-Length: 0\r\n\r\n", HOST);


  lastData = time(0);
  for (;;) {
    char buf[2048];
    int chunkSize = 0;
    unsigned long dataReady = 0;

    Sleep(1);

    if (ioctlsocket(soc, FIONREAD, &dataReady) < 0) break;
    if (dataReady) {
      chunkSize = recv(soc, buf, sizeof(buf), 0);
      if (chunkSize<0) {
        printf("Error: recv failed for client %i\r\n", (int)clientNo);
        break;
      } else if (!isBody) {
        char * headEnd = strstr(buf,"\xD\xA\xD\xA");
        if (headEnd) {
          headEnd+=4;
          chunkSize -= ((int)headEnd - (int)buf);
          if (chunkSize>0) {
            totalData += chunkSize;
            lastData = time(0);
            //fwrite(headEnd,1,got,STORE);
          }
          isBody=1;
        }
      } else {
        totalData += chunkSize;
        lastData = time(0);
        //fwrite(buf,1,got,STORE);
      }
    } else {
      time_t current = time(0);
      if (difftime(current, lastData) > timeOut) break;
    }
  }

  closesocket(soc);

  EnterCriticalSection(&cs);
  if (isTest) {
    expectedData = totalData;
  } else if (totalData != expectedData) {
    printf("Error: Client %u got %u bytes instead of %u\r\n", (int)clientNo, totalData, expectedData);
    bad++;
  } else {
    good++;
  }
  LeaveCriticalSection(&cs);

  return 0;
}


void RunMultiClientTest(int loop) {

  HANDLE hThread[CLIENTCOUNT] = {0};
  int i;

  for (i=0;i<CLIENTCOUNT;i++) {
    DWORD dummy;
    hThread[i] = CreateThread(NULL, 1024*32, (LPTHREAD_START_ROUTINE)ClientMain, (void*)(1000*loop+i), 0, &dummy);
  }

  WaitForMultipleObjects(CLIENTCOUNT, hThread, TRUE, 15000);
  for (i=0;i<CLIENTCOUNT;i++) {
    if (WaitForSingleObject(hThread[i], 0)==WAIT_OBJECT_0) {
      CloseHandle(hThread[i]);
      hThread[i]=0;
    }
  }
  for (i=0;i<CLIENTCOUNT;i++) {
    if (hThread[i]) {
      EnterCriticalSection(&cs);
      SuspendThread(hThread[i]); // -> check this thread in the debugger
      printf("Thread %i did not finish!\r\n", (int)(1000*loop+i));
      LeaveCriticalSection(&cs);
    }
  }
  EnterCriticalSection(&cs);
  printf("Test cylce %u completed\r\n\r\n", loop);
  LeaveCriticalSection(&cs);
}


int MultiClientTestAutomatic(unsigned long initialPostSize) {

  FILE        * log;
  int           cycle;

  postSize = initialPostSize;

  do {
    printf("Preparing test with %u bytes of data ...", postSize);
    ClientMain(0);
    if (expectedData==0) {
      printf(" Error: Could not read any data\a\r\n");
      return 1;
    }
    printf(" OK: %u bytes of data\r\n", expectedData);
    printf("Starting multi client test: %i cycles, %i clients each\r\n\r\n", (int)TESTCYCLES, (int)CLIENTCOUNT);
    good=bad=0;

    for (cycle=1;cycle<=TESTCYCLES;cycle++) {
      RunMultiClientTest(cycle);
    }

    printf("\r\n--------\r\n%u errors\r\n%u OK\r\n--------\r\n\r\n", bad, good);
    log = fopen("testclient.log", "at");
    if (log) {
      fprintf(log, "%u\t%u\t%u\r\n", postSize, good, bad);
      fclose(log);
    }

    postSize = (postSize!=0) ? (postSize<<1) : 1;

  } while (postSize!=0);

  return 0;
}


int SingleClientTestAutomatic(void) {

  FILE        * log;
  int           cycle;
  int           i;

  postSize = 0;
  for (cycle=0;;cycle++) {
    good=bad=0;
    for (i=0;i<1000;i++) {
      expectedData=3;
      ClientMain((void*)1);
    }
    log = fopen("testclient.log", "at");
    if (log) {
      fprintf(log, "Cylce<%u>\t%u\t%u\r\n", cycle, good, bad);
      fclose(log);
    }
    printf("test cycle %u: %u good, %u bad\r\n", cycle, good, bad);
  }

  return 0;
}


int main(int argc, char * argv[]) {

  WSADATA       wsaData = {0};
  HOSTENT     * lpHost = 0;

  if (WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR) {
    printf("\r\nCannot init WinSock\a\r\n");
    return 1;
  }

  lpHost = gethostbyname(HOST);
  if (lpHost == NULL) {
    printf("\r\nCannot find host %s\a\r\n",HOST);
    return 2;
  }

  target.sin_family = AF_INET;
  target.sin_addr.s_addr = *((u_long FAR *) (lpHost->h_addr));
  target.sin_port = htons(PORT);

  GetProcessAffinityMask(GetCurrentProcess(), &availableCPUs, &totalCPUs);
  printf("CPUs (bit masks): process=%x, system=%x\r\n", availableCPUs, totalCPUs);

  InitializeCriticalSectionAndSpinCount(&cs, 100000);

  /* Do the actual test here */
  MultiClientTestAutomatic(200000);
  //SingleClientTestAutomatic();

  /* Cleanup */
  DeleteCriticalSection(&cs);
  WSACleanup();
  return 0;
}
