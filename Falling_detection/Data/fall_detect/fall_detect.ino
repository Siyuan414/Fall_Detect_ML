
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SSD1306.h>
#include <Chirale_TensorFlowLite.h>

// include static array definition of pre-trained model
#include "fall_detector_cnn_Fl32_50.h"

// This TensorFlow Lite Micro Library for Arduino is not similar to standard
// Arduino libraries. These additional header files must be included.
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Globals pointers, used to address TensorFlow Lite components.
// Pointers are not usual in Arduino sketches, future versions of
// the library may change this...
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// There is no way to calculate this parameter
// the value is usually determined by trial and errors
// It is the dimension of the memory area used by the TFLite interpreter
// to store tensors and intermediate results
constexpr int kTensorArenaSize = 50 * 1024;

// Keep aligned to 16 bytes for CMSIS (Cortex Microcontroller Software Interface Standard)
// alignas(16) directive is used to specify that the array 
// should be stored in memory at an address that is a multiple of 16.

 uint8_t tensor_arena[kTensorArenaSize];

// Display config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// MPU6050 config
Adafruit_MPU6050 mpu;

//input buffer
float input_buffer[50][6];
int buffer_index = 0;

void setup() {
  // Initialize serial communications and wait for Serial Monitor to be opened
  Serial.begin(115200);
  while(!Serial);

  // Init OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Init MPU6050
  if (!mpu.begin()) {
    display.println("MPU6050 not found!");
    display.display();
    while (true);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  display.println("MPU6050 + OLED Ready");
  display.display();
  delay(1000);
  display.clearDisplay();

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_model);

  // Check if model and library have compatible schema version,
  // if not, there is a misalignement between TensorFlow version used
  // to train and generate the TFLite model and the current version of library
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model provided and schema version are not equal!");
    while(true); // stop program here
  }

  // This pulls in all the TensorFlow Lite operators.
  static tflite::AllOpsResolver resolver;

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  // if an error occurs, stop the program.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    while(true); // stop program here
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output = interpreter->output(0);
  for (int i = 0; i < input->dims->size; i++) {
  Serial.print(input->dims->data[i]);
  Serial.print(" ");
}

  display.println("Model loaded");
  display.display();
  delay(1000);

}

void runinference(){
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    Serial.println("Inference failed");
    return;
  }

}
void loop() {
  
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  input_buffer[buffer_index][0] = a.acceleration.x;
  input_buffer[buffer_index][1] = a.acceleration.y;
  input_buffer[buffer_index][2] = a.acceleration.z;
  input_buffer[buffer_index][3] = g.gyro.x;
  input_buffer[buffer_index][4] = g.gyro.y;
  input_buffer[buffer_index][5] = g.gyro.z;
  buffer_index++;
    
  if(buffer_index >= 50){
    float* input_data = interpreter->typed_input_tensor<float>(0);
    for (int i = 0; i < 50; i++) {
      for (int j = 0; j < 6; j++) {
        input_data[i * 6 + j] = input_buffer[i][j];
      }
    }

    //runinference();
    buffer_index = 0;
  }
  
  
 
  
  for (int i = 0; i < 4; i++) {
  display.print("Class "); display.print(i); display.print(": ");
  display.println(output->data.f[i], 6);
}
  delay(500);

  
}