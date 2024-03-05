#include <Adafruit_NeoPixel.h>

#define LEDSTRIP_PIN 6
#define BUTTON_PIN 2
#define LED_COUNT 20

const int MIC_PIN = A0;

int red;
int green;
int blue;
int in[128];
byte NoteV[13] = { 8, 23, 40, 57, 76, 96, 116, 138, 162, 187, 213, 241, 255 }; // data for note detection based on frequency
float InNotes[5];
float noteFreq[12];
float f_peaks[5]; 
int note;
int actualValue;
int j, k;
int selectedNoteIndex, upperLimitNoteIndex, selectedNote;
boolean newButtonState, oldButtonState = LOW;
bool listen = false;

Adafruit_NeoPixel strip(LED_COUNT, LEDSTRIP_PIN, NEO_GRB + NEO_KHZ800);
  
void setup() {
  strip.begin();           
  strip.setBrightness(10); 
  strip.clear(); 
  pinMode(BUTTON_PIN, INPUT);
}

void loop() {

  readButton();

  if (listen)
  {
    toneDetermiantion();

    actualValue = map(selectedNoteIndex, 0, 11, 2, 20);

    listenAnimation();
  }
  else
  {
    idleAnimation();
  }
}

void readButton()
{
  newButtonState = digitalRead(BUTTON_PIN);

  if (newButtonState == LOW && oldButtonState == HIGH) 
  {
    delay(20);

    newButtonState = digitalRead(BUTTON_PIN);

    if (newButtonState == LOW) 
    {
      listen = !listen;

      if (listen)
      {
        selectedNoteIndex = 0;
      }
    }
  } 

  oldButtonState = newButtonState;
}

void listenAnimation()
{
  strip.clear();
  
  for(int i = 0; i < actualValue; i++)
  {
    strip.setPixelColor(i, red, green, blue);
    strip.show();  
    delay(50);
  }
}

void idleAnimation()
{ 
  strip.clear();

  for(int i = 0; i < LED_COUNT; i++)
  {
    red = (red + 70) % 255;
    green = (green + 60) % 255;
    blue = (blue + 50) % 255;
    strip.setPixelColor(i, red, green, blue);
    strip.show();  
  }

  delay(200);
}

void toneDetermiantion()
{ 
  long unsigned int startTime, endTime;
  float analogValue;
  float sum1 = 0, sum2 = 0;
  float averageSum = 0, rootMeanSquareSum = 0;
  float samplingFreq;

  startTime = micros();

  for(int i = 0; i < 128 ; i++)
  {
      analogValue = analogRead(MIC_PIN) - 500;    
      sum1 += analogValue;                         
      sum2 += analogValue * analogValue;            
      analogValue *= (sin(i*3.14/128) * sin(i*3.14/128));  
      in[i] = 10 * analogValue;                         
      delayMicroseconds(195);                             
  }

  endTime = micros();

  averageSum = sum1 / 128;                                 
  rootMeanSquareSum = sqrt(sum2 / 128);                           
  samplingFreq = 128000000 / (endTime - startTime);    

  if (rootMeanSquareSum - averageSum > 3)
  {  
    FFT(128, samplingFreq);                          
  
    for (int i = 0; i < 12; i++)                   
    {
      noteFreq[i] = 0;
    }  

    for (int i = 0; i < 5; i++)
    {
      convertFreqToNote(i);

      for (int k = 0; k < 13; k++)
      { 
        if (InNotes[i] < NoteV[k]) {  
            upperLimitNoteIndex = k;
            break;
        }
      }

      if (upperLimitNoteIndex == 12)
      {
        upperLimitNoteIndex = 0;
      }

      noteFreq[upperLimitNoteIndex] = noteFreq[upperLimitNoteIndex] + (5 - i);
    }

    selectedNote = 0;

    for (int i = 0; i < 12; i++)  
    {
      if (selectedNote < noteFreq[i]) 
      {
        selectedNote = noteFreq[i];
        selectedNoteIndex = i;
      }  
    }

     switch (selectedNoteIndex)
    {
      case 0:       // do
      case 1:       // do #
        red = 204; green = 0; blue = 0;     // red
        break;
      case 2:       // re
      case 3:       // re #
        red = 230; green = 90; blue = 0;   // orange
        break;
      case 4:       // mi
        red = 255; green = 255; blue = 0;   // yellow
        break;
      case 5:       // fa
      case 6:       // fa #
        red = 102; green = 204; blue = 0;   // green
        break;
      case 7:       // sol
      case 8:       // sol #
        red = 0; green = 76; blue = 153;    // blue
        break;
      case 9:       // la
      case 10:      // la #
        red = 76; green = 0; blue = 153;    // purple
        break;
      case 11:      // si
        red = 153; green = 0; blue = 76;    // pink
        break;
      default:
        break;
    }  
  }
  else 
  {
    selectedNoteIndex = 0;
    red = 150; green = 150; blue = 150;
  }
}

void convertFreqToNote(int i)
{
  if(f_peaks[i] > 1040) { f_peaks[i] = 0; InNotes[i] = 0; }
  if(f_peaks[i] >= 65.4 && f_peaks[i] <= 130.8) { f_peaks[i] = 255 * ((f_peaks[i] / 65.4) - 1); InNotes[i] = f_peaks[i]; }
  if(f_peaks[i] >= 130.8 && f_peaks[i] <= 261.6) { f_peaks[i] = 255 * ((f_peaks[i] / 130.8) - 1); InNotes[i] = f_peaks[i]; }
  if(f_peaks[i] >= 261.6 && f_peaks[i] <= 523.25) { f_peaks[i] = 255 * ((f_peaks[i] / 261.6) - 1); InNotes[i] = f_peaks[i]; }
  if(f_peaks[i] >= 523.25 && f_peaks[i] <= 1046)  { f_peaks[i] = 255 * ((f_peaks[i] / 523.25) - 1); InNotes[i] = f_peaks[i]; }
  if(f_peaks[i] >= 1046 && f_peaks[i] <= 2093)  { f_peaks[i] = 255 * ((f_peaks[i] / 1046) - 1); InNotes[i] = f_peaks[i]; }
  if(f_peaks[i] > 255) { f_peaks[i] = 254; InNotes[i] = 254; }
}

float FFT(byte N, float Frequency)
{
  byte data[8]={1, 2, 4, 8, 16, 32, 64, 128};
  int a, c1, f, o, x;
  a = N;  
                                  
  for(int i=0;i<8;i++)                 //calculating the levels
    { if(data[i]<=a){o=i;} }
  o=7;
  byte in_ps[data[o]]={};     //input for sequencing
  float out_r[data[o]]={};   //real part of transform
  float out_im[data[o]]={};  //imaginory part of transform
            
  x=0;  
        for(int b=0;b<o;b++)                     // bit reversal
          {
            c1=data[b];
            f=data[o]/(c1+c1);
                  for(int j=0;j<c1;j++)
                      { 
                      x=x+1;
                      in_ps[x]=in_ps[j]+f;
                      }
          }
  
        for(int i=0;i<data[o];i++)            // update input array as per bit reverse order
          {
            if(in_ps[i]<a)
            {out_r[i]=in[in_ps[i]];}
            if(in_ps[i]>a)
            {out_r[i]=in[in_ps[i]-a];}      
          }

  int i10,i11,n1;
  float e,c,s,tr,ti;

      for(int i=0;i<o;i++)                                    //fft
      {
      i10=data[i];              // overall values of sine cosine  
      i11=data[o]/data[i+1];    // loop with similar sine cosine
      e=6.283/data[i+1];
      e=0-e;
      n1=0;

            for(int j=0;j<i10;j++)
            {
            c=cos(e*j); 
            s=sin(e*j); 
            n1=j;
            
                  for(int k=0;k<i11;k++)
                  {
                  tr=c*out_r[i10+n1]-s*out_im[i10+n1];
                  ti=s*out_r[i10+n1]+c*out_im[i10+n1];
            
                  out_r[n1+i10]=out_r[n1]-tr;
                  out_r[n1]=out_r[n1]+tr;
            
                  out_im[n1+i10]=out_im[n1]-ti;
                  out_im[n1]=out_im[n1]+ti;          
            
                  n1=n1+i10+i10;
                    }       
              }
      }

  //---> here onward out_r contains amplitude and our_in conntains frequency (Hz)
      for(int i=0;i<data[o-1];i++)               // getting amplitude from compex number
          {
          out_r[i]=sqrt((out_r[i]*out_r[i])+(out_im[i]*out_im[i])); // to  increase the speed delete sqrt
          out_im[i]=(i*Frequency)/data[o];
          /*
          Serial.print(out_im[i],2); Serial.print("Hz");
          Serial.print("\t");                            // uncomment to print freuency bin    
          Serial.println(out_r[i]); 
          */
          }

  x=0;       // peak detection
    for(int i=1;i<data[o-1]-1;i++)
        {
        if(out_r[i]>out_r[i-1] && out_r[i]>out_r[i+1]) 
        {in_ps[x]=i;    //in_ps array used for storage of peak number
        x=x+1;}    
        }

  s=0;
  c=0;
      for(int i=0;i<x;i++)             // re arraange as per magnitude
      {
          for(int j=c;j<x;j++)
          {
              if(out_r[in_ps[i]]<out_r[in_ps[j]]) 
                  {s=in_ps[i];
                  in_ps[i]=in_ps[j];
                  in_ps[j]=s;}
          }
      c=c+1;
      }
      
      for(int i=0;i<5;i++)     // updating f_peak array (global variable)with descending order
      {
      f_peaks[i]=(out_im[in_ps[i]-1]*out_r[in_ps[i]-1]+out_im[in_ps[i]]*out_r[in_ps[i]]+out_im[in_ps[i]+1]*out_r[in_ps[i]+1])
      /(out_r[in_ps[i]-1]+out_r[in_ps[i]]+out_r[in_ps[i]+1]);
      }
}