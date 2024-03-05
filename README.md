# 한옥 온, 습도 자동 제어 시스템
학부 졸업작품(캡스톤 디자인, 팀 프로젝트)

### 진행 기간 
- 22.09.05 ~ 23.05.22

### 역할
- 제어 소프트웨어 구현
- 제어 앱 구현

### 개발 환경
- IDE : Arduino IDE
- 앱 : MIT App inventor

### 프로젝트 설명
- 스마트폰 제어 앱으로 한옥 하드웨어 및 온, 습도 제어
- 블루투스 모듈을 통해 스마트폰, 아두이노 간 통신 연결
  
#### 제어 회로 및 하드웨어
<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/21c9bd3c-73b6-40ea-b21c-54bfeef08a02">
</p>
<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/23335349-d149-46d0-ac7f-64201164b86b">
</p>

- 블루투스 모듈을 통해 센서의 온, 습도 값 스마트폰 앱으로 전송
- 서보 모터로 창문 제어
- 릴레이로 환풍기 및 발열패드, 조명 제어
- 서보 모터 전용 DC 6V, 발열패드 전용 220V 등 외부 전원 사용

#### 스마트폰 앱 & 아두이노 연동
```
  while(btSerial.available()){//스마트폰(블루투스 시리얼)에서 온 정보 있을 시 
    order=(char)btSerial.read();//문자형으로 수신
    orderAll+=order; //문자열에 가산하여 수신전문 완성
    delay(5);
    } 
        
  if(!orderAll.equals("")){//스마트폰에서 온 정보가 있을 시
    Serial.print(orderAll); //아두이노 시리얼에 출력
    }

 if(orderAll=="f"){// 환풍기 on
          fan=true;
          }
         if(orderAll=="n"){// 환풍기 off
          fan=false;
          }

void fanControl(boolean fan){//환풍기 제어 함수
  if(fan){//환풍기 on
    digitalWrite(FAN_PIN,HIGH); //fan On
    }else{//환풍기 off
      digitalWrite(FAN_PIN,LOW); //fan Off
      }
  }
```
- 각 제어 별 이니셜 지정
- 아두이노에서 해당 이니셜 수신 시 판별 후 제어

#### 발열패드 제어
<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/0e2d58fc-a33a-4b33-9026-ca0f9eb44d1c">
</p>

```
if(temp>38){//온도가 38도 를 초과할 시
          heatEnable = false;
          heater = false;
          }else{
            heatEnable = true;
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
            
```
- 수동 제어 및 자동 제어 시 일정 온도 또는 목푯값 초과 시 발열패드 인터락 기능 구현

#### 자동제어
<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/727f0799-45f2-4db6-9cf2-752bae9253b2">
</p>

```
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
```
- 자동 제어 전환 시 온, 습도 현재 값과 목푯값 비교 후 제어 수행 -> 습도, 온도 목푯값 일정 범위 내에서 조정 가능

#### 수동제어
<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/ee1a928d-8331-4938-bfad-82f53a696871">
</p>
 
```
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
```
- 스마트폰 앱에서 창문, 환풍기, 발열 패드, 조명 조작 가능
- Arduino Servo library의 read 함수와 조건문 이용하여 2개의 창문 개별 조작 기능 구현
- Attach, detach 함수 이용하여 서보 모터 떨림 현상 해결

### 프로젝트 성과
- 온, 습도 센서 기준 값 제어 시스템 구현
- 서보모터, 릴레이, 발열패드, 팬 등 하드웨어 제어
- 제어 알고리즘 구현
- 제어 회로 구성

### 프로젝트 결과 소개
#### 수동제어
<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/db2b5ccc-1c89-44ea-a526-f205b6db52bb">
</p>

<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/1ae9ac38-0646-458a-90be-9610bebf401d">
</p>

<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/cc762c2e-7758-4cd4-8a22-714e765aec39">
</p>

<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/ab4713db-c370-4253-91d6-9f27cb8d163a">
</p>

#### 자동제어
<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/bae8d818-e864-4560-9e59-f6a79dda28a5">
</p>

<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/0f239c19-3e1b-4c42-a7c5-ac44bd5e8ade">
</p>

<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/e513a082-39b6-4ed5-9fad-3b753743f6c7">
</p>

<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/1fb43ecd-7920-4b4c-8102-67a7725b8535">
</p>

<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/b9738d51-d927-4206-a089-efcd584de7b0">
</p>

<p align="center">
  <img src="https://github.com/madorosjang/hanok_temp_n_humid_auto_control_system/assets/122807795/4a76134a-799c-4a07-b6be-2769b53eafcd">
</p>

### 프로젝트 후 배운 점
- 협업은 서로 다른 능력의 상호작용을 통해 이뤄짐을 깨달음
- 제어 알고리즘 구현 지식 습득
  
