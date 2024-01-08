// 헤더파일
#include <SoftwareSerial.h> //시리얼 통신 라이브러리 호출
#include <Servo.h> //서보 라이브러리 호출
#include <DHT.h> //습도센서 라이브러리 호출

//전처리
#define LIGHT_PIN 4 // 조명 제어 pin 번호
#define AUTO_INDICATOR 7 //auto mode indicator pin 번호
#define HEATER_PIN 8 // heater 제어 pin 번호
#define LEFT_PIN 10 //좌측 서보모터용 디지털 I/O pin 
#define RIGHT_PIN 11 //우측 서보모터용 디지털 I/O pin 
#define DHT_PIN 12 //온습도센서용 디지털 I/O pin
#define FAN_PIN 13 // fan 제어 pin 번호
#define DHT_TYPE DHT11 //온습도 센서 타입 정의
#define OPEN_RIGHT_ANGLE 139.0 //오른쪽 서보모터 여는 각도
#define OPEN_LEFT_ANGLE 139.0 //왼쪽 서보모터 여는 각도
#define CLOSE_RIGHT_ANGLE 175 //오른쪽 서보모터 닫는 각도
#define CLOSE_LEFT_ANGLE 180 //왼쪽 서보모터 닫는 각도
#define OPEN_FOR_LOOP 10 //열때 루프 횟수
#define CLOSE_FOR_LOOP 4 //닫을때 푸르 횟수
#define WINDOW_DELAY 150 //창문 움직일때 딜레이

DHT dht(DHT_PIN,DHT_TYPE); //온습도 센서 객체 생성
Servo leftServo; //좌측 창문 제어 위한 서보객체 생성
Servo rightServo; //우측 창문 제어 위한 서보객체 생성
SoftwareSerial btSerial(2,3); // 아두이노의 RX, TX에 블루투스 모듈의 TX,RX를 연결

char order; //아두이노가 수신하는 문자
String orderAll=""; //수신한 문자 -> 문자열 변환
int temp, humid; //온도 습도 초기화
int humidTarget=45,tempTargetS=20,tempTargetW=26; // 습도 목표값 45%, 온도 목표값은 계절에 따라 여름엔 20도, 겨울엔 26도로 설정.
boolean autoMode = false; // 자동제어 여부 확인
boolean window = false; // 전체 창문 상태 확인
boolean right_window = false; // 우측 창문
boolean left_window = false; //좌측 창문
boolean heater = false; // 히터 상태 확인
boolean fan = false; // 환풍기 상태 확인
boolean light = false; // 조명 상태 확인
boolean summer = false; //여름인지 겨울인지 확인
boolean heatEnable = true; //온도 목표값에 따라 히터 동작 여부 결정(현재 온도가 목표값 보다 높다면 히터 동작 불가)
float  openRight[OPEN_FOR_LOOP] = {165.0,160.0,155.0,150.0,145.0,143.0,142.0,140.0,139.5,OPEN_RIGHT_ANGLE}; //오른쪽 서보모터 열때 
float  openLeft[OPEN_FOR_LOOP] = {165.0,160.0,155.0,150.0,145.0,142.0,140.0,139.8,139.6,OPEN_LEFT_ANGLE}; //왼쪽 서보모터 여는 열때 
int closeRight[CLOSE_FOR_LOOP] = {150,165,174,CLOSE_RIGHT_ANGLE}; //오른쪽 서보모터 닫을때
int closeLeft[CLOSE_FOR_LOOP] = {150,165,170,CLOSE_LEFT_ANGLE}; // 왼쪽 서보모터 닫을때

//사용자 지정 함수
void dht11(); //스마트폰에 온습도를 출력하기 위한 함수
void windowControl(boolean window);//모든 창문제어함수
void rightWindowControl(boolean right_window); //오른쪽 창문 제어 함수
void leftWindowControl(boolean left_window); //왼쪽 창문 제어 함수
void fanControl(boolean fan); //환풍기 제어 함수
void lightControl(boolean light); //조명 제어 함수
void humidControl(int humid, int humidTargetget); //현재 습도와 목표값 비교 후 실행
void tempControl(int temp, int tempTarget); //현재 온도와 목표값 비교 후 실행
void heaterControl(boolean heater,boolean heatEnable); //히터 제어 함수

void setup() {
 Serial.begin(9600);  // 아두이노의 시리얼 전송속도(BAUD Rate)와 블루투스의 데이터 전송속도를 같게 해 줌
 btSerial.begin(9600); //블루투스의 시리얼 전송속도 동기화
 leftServo.attach(LEFT_PIN); //서보 시그널 핀설정
 leftServo.write(CLOSE_LEFT_ANGLE); //서보 초기각도 설정
 rightServo.attach(RIGHT_PIN); //서보 시그널 핀설정
 rightServo.write(CLOSE_RIGHT_ANGLE); //서보 초기각도 설정
 delay(WINDOW_DELAY);
 leftServo.detach();
 rightServo.detach();
 pinMode(LIGHT_PIN,OUTPUT); //lightpin을 릴레이 출력으로 설정.
 pinMode(FAN_PIN,OUTPUT); //fanpin을 릴레이 출력으로 설정.
 pinMode(HEATER_PIN,OUTPUT); //HEATER_PIN을 릴레이 출력으로 설정.
 pinMode(AUTO_INDICATOR,OUTPUT); //AUTO_INDICATOR를 릴레이 출력으로 설정.
}
    
void loop() {

  dht11(); //스마트폰에 온습도 출력
  
  while(btSerial.available()){//스마트폰(블루투스 시리얼)에서 온 정보 있을 시 
    order=(char)btSerial.read();//문자형으로 수신
    orderAll+=order; //문자열에 가산하여 수신전문 완성
    delay(5);
    } 
        
  if(!orderAll.equals("")){//스마트폰에서 온 정보가 있을 시
    Serial.print(orderAll); //아두이노 시리얼에 출력
    }
    
  if(orderAll=="a"){//자동제어 여부 확인
      autoMode=true;
      humidTarget=45;
      if(summer){//여름일시
        tempTargetS=20;
        }else{//겨울일시
          tempTargetW=26;
          }
      }

    if(orderAll=="x"){//날짜가 여름일 시(4~9월)
      summer=true;
      }      

    if(orderAll=="y"){//날짜가 겨울일 시(10~3월)
      summer=false;
      }
      
    if(orderAll=="m"){//수동제어로 변환할 시
      autoMode=false;
      fan=false;
      heater=false;
      window = false;
      humidTarget=45;
      if(summer){//여름일시
        tempTargetS=20;
        }else{//겨울일시
          tempTargetW=26;
          }
      windowControl(window);
      fanControl(fan);
      heaterControl(heater,heatEnable);
      }
    if(orderAll=="r"){//블루투스 연결 해제 시 -> 초기화
      humidTarget=45;
      autoMode=false;
      fan=false;
      heater=false;
      window = false;
      light=false;
      if(summer){//여름일시
        tempTargetS=20;
        }else{//겨울일시
          tempTargetW=26;
          }
      windowControl(window);
      fanControl(fan);
      heaterControl(heater,heatEnable);
      lightControl(light);
      }
              
    if(autoMode){//자동제어일시
      digitalWrite(AUTO_INDICATOR,HIGH); //AUTO_INDICATOR relay on
      if(orderAll=="s"){//온도 목표값 상승 시
        if(summer){//여름일 시
          if(tempTargetS<26){//범위 내인 경우
            tempTargetS=tempTargetS+1;
            }else{//범위 밖인 경우
              tempTargetS=tempTargetS;
              } 
          tempControl(temp,tempTargetS);  
          }else{//겨울일 시
            if(tempTargetW<32){//범위 내인 경우
              tempTargetW=tempTargetW+1;
              }else{//범위 밖인 경우
                tempTargetW=tempTargetW;
                }
              tempControl(temp,tempTargetW); 
            }
        }
      if(orderAll=="u"){//온도 목표값 감소 시
        if(summer){//여름일 시
          if(tempTargetS>18){//범위 내인 경우
            tempTargetS=tempTargetS-1;
            }else{//범위 밖인 경우
              tempTargetS=tempTargetS;
              }   
            tempControl(temp,tempTargetS);    
          }else{//겨울일 시
            if(tempTargetW>22){//범위 내인 경우
              tempTargetW=tempTargetW-1;
              }else{//범위 밖인 경우
                tempTargetW=tempTargetW;
                }    
              tempControl(temp,tempTargetW);
            }
        }
      //온도 목표값 변화 없을 시
      if(summer){//여름일 시
        tempControl(temp,tempTargetS);
        }else{//겨울일 시
          tempControl(temp,tempTargetW);
          }
       
      if(orderAll=="p"){//습도 목표값 상승 시 
        if(humidTarget<55){//범위 내인 경우
          humidTarget=humidTarget+1;
          }else{//범위 밖인 경우
            humidTarget=humidTarget;
            }
        humidControl(humid,humidTarget);
        }
      if(orderAll=="i"){//습도 목표값 감소 시
        if(humidTarget>35){//범위 내인 경우
          humidTarget=humidTarget-1;
          }else{//범위 밖인 겨우
             humidTarget=humidTarget;
            }
        humidControl(humid,humidTarget);    
        }
      //습도 목표값 변화 없을 시
      humidControl(humid,humidTarget);
      if(orderAll=="l"){// 조명 on, 조명은 자동제어 상태에서도 제어가능
          light=true;
          }
      if(orderAll=="t"){// 조명 off
          light=false;
          }
      lightControl(light);    
      }else{//수동제어일시
         digitalWrite(AUTO_INDICATOR,LOW); //AUTO_INDICATOR relay off
         if(temp>38){//온도가 38도 를 초과할 시
          heatEnable = false;
          heater = false;
          }else{
            heatEnable = true;
            }
         if(orderAll=="o"){//모든창문 열기
          window = true;
          windowControl(window);
          }
         if(orderAll=="c"){//모든창문 닫기
          window = false;
          windowControl(window);
          }
         if(orderAll=="or"){//오른쪽 창문 열기
          right_window=true;
          rightWindowControl(right_window);
          }
         if(orderAll=="cr"){//오른쪽 창문 닫기
          right_window=false;
          rightWindowControl(right_window);
          }
         if(orderAll=="ol"){//왼쪽 창문 열기
          left_window=true;
          leftWindowControl(left_window);
          }
         if(orderAll=="cl"){//왼쪽 창문 닫기
          left_window=false;
          leftWindowControl(left_window);
          }
         if(orderAll=="l"){// 조명 on
          light=true;
          }
         if(orderAll=="t"){// 조명 off
          light=false;
          }
         if(orderAll=="f"){// 환풍기 on
          fan=true;
          }
         if(orderAll=="n"){// 환풍기 off
          fan=false;
          }
         if(orderAll=="h"){// 히터 on
          heater=true;
          }
         if(orderAll=="e"){// 히터 off
          heater=false;
          }
        heaterControl(heater, heatEnable);
        fanControl(fan);
        lightControl(light);
        }//수동제어 끝부분   
      orderAll=""; //명령 초기화      
  delay(1000);
}

void dht11(){ //스마트폰에 온습도를 출력하기 위한 함수
  temp = dht.readTemperature(); //온도값 초기화
  humid = dht.readHumidity(); //습도값 
  btSerial.print(temp); //블루투스 시리얼에 현재 온도값 출력
  btSerial.print(humid); //블루투스 시리얼에 현재 습도값 출력
  }

void windowControl(boolean window){//모든 창문제어함수
  if(window){//모든창문열기
    if(leftServo.read()<=OPEN_LEFT_ANGLE && rightServo.read() <= OPEN_RIGHT_ANGLE){ //모든 창문 열렸는지 확인
        leftServo.detach();//연결해제
        rightServo.detach();//연결해제
        }else if(leftServo.read()==CLOSE_LEFT_ANGLE && rightServo.read()<=OPEN_RIGHT_ANGLE){//왼쪽만 열어야할때
           leftServo.attach(LEFT_PIN);        
           for(int i=0;i<OPEN_FOR_LOOP;i++){
              leftServo.write(openLeft[i]);
              delay(WINDOW_DELAY);
            }
           leftServo.detach();//연결해제
        }else if(leftServo.read()<=OPEN_LEFT_ANGLE && rightServo.read()==CLOSE_RIGHT_ANGLE){//오른쪽만 열어야할때
           rightServo.attach(RIGHT_PIN);        
           for(int i=0;i<OPEN_FOR_LOOP;i++){
              rightServo.write(openRight[i]);
              delay(WINDOW_DELAY);
            }
           rightServo.detach();//연결해제
        }else{//전부 열어야할때
           leftServo.attach(LEFT_PIN);
           rightServo.attach(RIGHT_PIN);        
           for(int i=0;i<OPEN_FOR_LOOP;i++){
              leftServo.write(openLeft[i]);
              rightServo.write(openRight[i]);
              delay(WINDOW_DELAY);
            }
           leftServo.detach();//연결해제
           rightServo.detach();//연결해제
        }
   }else{//모든 창문 닫기
       if(leftServo.read()==CLOSE_LEFT_ANGLE && rightServo.read()== CLOSE_RIGHT_ANGLE){//모든 창문 닫혔는지 확인
          leftServo.detach();//연결해제
          rightServo.detach();//연결해제
          }else if(leftServo.read()<=OPEN_LEFT_ANGLE && rightServo.read()==CLOSE_RIGHT_ANGLE){//왼쪽만 닫아야 할때
           leftServo.attach(LEFT_PIN);        
           for(int i=0;i<CLOSE_FOR_LOOP;i++){
              leftServo.write(closeLeft[i]);
              delay(WINDOW_DELAY);
            }
           leftServo.detach();//연결해제
        }else if(leftServo.read()==CLOSE_LEFT_ANGLE && rightServo.read()<=OPEN_RIGHT_ANGLE){//오른쪽만 닫아야 할때
           rightServo.attach(RIGHT_PIN);        
           for(int i=0;i<CLOSE_FOR_LOOP;i++){
              rightServo.write(closeRight[i]);
              delay(WINDOW_DELAY);
            }
           rightServo.detach();//연결해제
        }else{//전부 닫아야 할때
           leftServo.attach(LEFT_PIN);
           rightServo.attach(RIGHT_PIN);        
           for(int i=0;i<CLOSE_FOR_LOOP;i++){
              leftServo.write(closeLeft[i]);
              rightServo.write(closeRight[i]);
              delay(WINDOW_DELAY);
            }
           leftServo.detach();//연결해제
           rightServo.detach();//연결해제
        }        
      }                   
  }

void rightWindowControl(boolean right_window){//오른쪽 창문 제어 함수
  if(right_window){//오른쪽 창문 열때
      if(rightServo.read()<=OPEN_RIGHT_ANGLE){//오른쪽 창문 열렸는지 확인
            rightServo.detach();//연결해제
          }else{//오른쪽 창문 열어야 할 때
            rightServo.attach(RIGHT_PIN);
            for(int i=0;i<OPEN_FOR_LOOP;i++){
              rightServo.write(openRight[i]);
              delay(WINDOW_DELAY);
              }
            rightServo.detach();//연결해제
            }
        }else{//오른쪽 창문 닫을때
        if(rightServo.read()==CLOSE_RIGHT_ANGLE){//오른쪽 창문 닫혔는지 확인
              rightServo.detach();//연결해제
            }else{//오른쪽 창문 닫아야 할 때
            rightServo.attach(RIGHT_PIN);
            for(int i=0;i<CLOSE_FOR_LOOP;i++){
              rightServo.write(closeRight[i]);
              delay(WINDOW_DELAY);
              }
            rightServo.detach();//연결해제
            }
        }
  }

void leftWindowControl(boolean left_window){//왼쪽 창문 제어 함수
  if(left_window){//왼쪽 창문 열때
          if(leftServo.read()<=OPEN_LEFT_ANGLE){//왼쪽 창문 열렸는지 확인
              leftServo.detach();//연결해제
                }else{//왼쪽 창문 열어야 할 때
                  leftServo.attach(LEFT_PIN);
                  for(int i=0;i<OPEN_FOR_LOOP;i++){
                    leftServo.write(openLeft[i]);
                    delay(WINDOW_DELAY);
                  }
                leftServo.detach();//연결해제
                }
          }else{//왼쪽 창문 닫을때
          if(leftServo.read()==CLOSE_LEFT_ANGLE){//왼쪽 창문 닫혔는지 확인
                leftServo.detach();//연결해제
              }else{//왼쪽 창문 닫아야 할 때
                leftServo.attach(LEFT_PIN);
                for(int i=0;i<CLOSE_FOR_LOOP;i++){
                  leftServo.write(closeLeft[i]);
                  delay(WINDOW_DELAY);
                }
              leftServo.detach();//연결해제
              }
        }
  }  


void fanControl(boolean fan){//환풍기 제어 함수
  if(fan){//환풍기 on
    digitalWrite(FAN_PIN,HIGH); //fan On
    }else{//환풍기 off
      digitalWrite(FAN_PIN,LOW); //fan Off
      }
  }

void lightControl(boolean light){//조명 제어 함수
  if(light){//조명 on
    digitalWrite(LIGHT_PIN,HIGH); //조명 On
    }else{//조명 off
      digitalWrite(LIGHT_PIN,LOW); //조명 Off
      }
  }    

void humidControl(int humid, int humidTargetget){//현재 습도와 목표값 비교 후 실행
  if(humid>humidTarget){//자동제어가 필요한 경우
      window = true;
      fan=true;
      windowControl(window);
      fanControl(fan);
    }else{ //자동제어 필요 없는 경우
      window=false;
      fan=false;
      windowControl(window);
      fanControl(fan);
      }
  }

void tempControl(int temp, int tempTarget){//현재 온도와 목표값 비교 후 실행
 if(temp<tempTarget){//자동제어 필요한 경우
    heater=true;
    }else{//자동제어 필요 없는 경우
      heater=false;
      }
  heaterControl(heater,heatEnable);
  }

void heaterControl(boolean heater,boolean heatEnable){//히터 제어 함수
  if(heatEnable){//히터 동작 가능한 경우
    if(heater){//히터 on
    digitalWrite(HEATER_PIN,HIGH); //heater On
    }else{//히터 off
      digitalWrite(HEATER_PIN,LOW); //heater Off
      }
    }else{//히터 동작 불가능한 경우
      digitalWrite(HEATER_PIN,LOW); //heater Off
      heater=false;
      }
  }
