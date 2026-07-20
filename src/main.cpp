#include <Arduino.h>
#include <M5Unified.h>
#include <Dynamixel2Arduino.h>
#include <vector>

#define DEBUG_SERIAL Serial

// M5Stack Core2 PORT A pin configuration
const uint8_t PIN_RX_SERVO = 33;
const uint8_t PIN_TX_SERVO = 32;

// DYNAMIXEL communication
HardwareSerial& DXL_SERIAL = Serial1;
Dynamixel2Arduino dxl(DXL_SERIAL);
const uint8_t NUM_BAUDS = 4;
const uint32_t BAUD_RATES[NUM_BAUDS] = {57600, 115200, 1000000, 2000000};

// DYNAMIXEL configuration
const uint8_t DXL_ID = 1;
const float DXL_PROTOCOL_VERSION = 2.0;

struct DynamixelInfo {
    uint32_t baudRate;
    uint8_t id;
};
// List of scan results
std::vector<DynamixelInfo> foundDynamixels;

// UI state management
enum AppState {
    MAIN_MENU,
    SCAN_MODE,
    CHANGE_ID_BAUD_MODE,
    EXAMPLE_EXECUTION_MODE
};
AppState currentState = MAIN_MENU;

// ID/Baud Change Mode variables
uint8_t currentBaudIndex = 0;
uint8_t newBaudIndex = 0;
uint8_t targetServoId = 0;
uint8_t newServoId = 0;

// Sample Execution Mode variables
uint8_t sampleServoId = 1;
uint8_t sampleBaudIndex = 0;

// Button dimensions for ID/Baud Change Mode
const int VALUE_WIDTH = 180;
const int ROW_HEIGHT = 40;
const int START_Y = 60;
unsigned long lastChangeTime = 0;
const unsigned long CHANGE_DELAY = 200; // 200ms delay between changes

// Scan mode button coordinates and size
const int SCAN_BUTTON_X = 100;
const int SCAN_BUTTON_Y = 150;
const int SCAN_BUTTON_WIDTH = 120;
const int SCAN_BUTTON_HEIGHT = 60;

// Arrow size
const int ARROW_WIDTH = 30;
const int ARROW_HEIGHT = 30;

// Button dimensions for Example Execution Mode
const int BUTTON_WIDTH = 150;
const int BUTTON_HEIGHT = 60;
const int BUTTON_Y = 160;

// Function Prototypes
void drawMainMenu();
void drawScanMode();
void drawArrow(int x, int y, bool isLeft);
void drawChangeIdBaudMode(uint32_t currentBaud, uint8_t targetServoId, uint8_t newServoId, uint32_t newBaud);
void drawSampleExecutionMode(uint32_t currentBaud, uint8_t sampleServoId);
void scanDynamixel();
void handleChangeIdBaudModeTouch();
void handleSampleExecutionModeTouch();
void handleScanMode();
void handleChangeIdBaudMode();
void handleSampleExecutionMode();

// Draw Main Menu
void drawMainMenu() {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("DYNAMIXEL Wizard M5Core2!");
    M5.Lcd.println("Button A: Scan");
    M5.Lcd.println("Button B: ID/Baud");
    M5.Lcd.println("Button C: Sample Exec");
}

// Draw Scan Mode
void drawScanMode() {
    // Title
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(60, 50);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("Scan Mode");

    // Draw scan button
    M5.Lcd.drawRect(SCAN_BUTTON_X, SCAN_BUTTON_Y, SCAN_BUTTON_WIDTH, SCAN_BUTTON_HEIGHT, TFT_WHITE);
    M5.Lcd.setCursor(SCAN_BUTTON_X + 20, SCAN_BUTTON_Y + 25);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("Start Scan");
}

// Draw Arrow
void drawArrow(int x, int y, bool isLeft) {
    M5.Lcd.fillTriangle(
        x + (isLeft ? ARROW_WIDTH : 0), y,
        x + (isLeft ? ARROW_WIDTH : 0), y + ARROW_HEIGHT,
        x + (isLeft ? 0 : ARROW_WIDTH), y + ARROW_HEIGHT / 2,
        TFT_WHITE
    );
}

// Draw Change ID/Baudrate Mode
void drawChangeIdBaudMode(uint32_t currentBaud, uint8_t targetServoId, uint8_t newServoId, uint32_t newBaud) {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_WHITE);

    // Title
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.println("Change ID/Baudrate Mode");

    // Baud Rate Row
    int y = 30;
    drawArrow(10, y, true);
    drawArrow(290, y, false);
    M5.Lcd.setCursor(50, y + 5);
    M5.Lcd.printf("Use Baud: %lu", currentBaud);

    // Target Servo ID Row
    y += 35;
    drawArrow(10, y, true);
    drawArrow(290, y, false);
    M5.Lcd.setCursor(50, y + 5);
    M5.Lcd.printf("Target ID: %d", targetServoId);

    // New Servo ID Row
    y += 35;
    drawArrow(10, y, true);
    drawArrow(290, y, false);
    M5.Lcd.setCursor(50, y + 5);
    M5.Lcd.printf("New ID: %d", newServoId);

    // New Baud Rate Row
    y += 35;
    drawArrow(10, y, true);
    drawArrow(290, y, false);
    M5.Lcd.setCursor(50, y + 5);
    M5.Lcd.printf("New Baud: %lu", newBaud);

    // Apply Buttons
    y += 52;
    M5.Lcd.drawRect(10, y, 150, 30, TFT_WHITE);
    M5.Lcd.setCursor(20, y + 5);
    M5.Lcd.print("Apply ID");

    M5.Lcd.drawRect(170, y, 150, 30, TFT_WHITE);
    M5.Lcd.setCursor(180, y + 5);
    M5.Lcd.print("Apply Baud");

    M5.Lcd.setCursor(10, 220);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("B: Back to Main Menu");
}

// Draw Sample Execution Mode
void drawSampleExecutionMode(uint32_t currentBaud, uint8_t sampleServoId) {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_WHITE);

    // Title
    M5.Lcd.setCursor(10, 0);
    M5.Lcd.println("Sample Execution Mode");

    // ID Row
    int y = 30;
    drawArrow(10, y, true);
    drawArrow(290, y, false);
    M5.Lcd.setCursor(50, y + 5);
    M5.Lcd.printf("ID: %d", sampleServoId);

    // Baud Rate Row
    y += 35;
    drawArrow(10, y, true);
    drawArrow(290, y, false);
    M5.Lcd.setCursor(50, y + 5);
    M5.Lcd.printf("Baud: %lu", currentBaud);

    // Execute Buttons
    M5.Lcd.drawRect(10, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, TFT_WHITE);
    M5.Lcd.setCursor(20, BUTTON_Y + 10);
    M5.Lcd.print("Position");
    M5.Lcd.setCursor(20, BUTTON_Y + 35);
    M5.Lcd.print("Mode");

    M5.Lcd.drawRect(170, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, TFT_WHITE);
    M5.Lcd.setCursor(180, BUTTON_Y + 10);
    M5.Lcd.print("Velocity");
    M5.Lcd.setCursor(180, BUTTON_Y + 35);
    M5.Lcd.print("Mode");

    M5.Lcd.setCursor(10, 220);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("C: Back to Main Menu");
}

// Scan DYNAMIXELs
void scanDynamixel() {
    foundDynamixels.clear();
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("Scanning DYNAMIXELs...\n");

    for (uint8_t index = 0; index < NUM_BAUDS; index++) {
        uint8_t foundCount = 0;

        dxl.begin(BAUD_RATES[index]);
        dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

        M5.Lcd.setCursor(10, 25 + index * 15);
        M5.Lcd.printf("Baud: %7ld", BAUD_RATES[index]);
        delay(5);

        for (uint8_t id = 0; id < DXL_BROADCAST_ID; id++) {
            if (dxl.ping(id)) {
                DynamixelInfo info = {BAUD_RATES[index], id};
                foundDynamixels.push_back(info);
                foundCount++;
                delay(5);
            }
        }
        M5.Lcd.printf(" --- %d found", foundCount);
    }
    delay(1000); // Wait for 1 second

    // Display the results
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("Scan Results:");

    int yOffset = 30;
    for (const auto& dxlInfo : foundDynamixels) {
        if (yOffset > 220) break; // Prevent overflow on display
        M5.Lcd.setCursor(10, yOffset);
        M5.Lcd.printf("Baud: %lu, ID: %d", dxlInfo.baudRate, dxlInfo.id);
        yOffset += 20; // Increase line spacing
    }

    M5.Lcd.setCursor(10, 220);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("A: Back to Main Menu");
}

// Handle Change ID/Baudrate Mode Touch Events
void handleChangeIdBaudModeTouch() {
    static int lastTouchedRow = -1;
    static bool lastTouchedLeft = false;
    static unsigned long touchStartTime = 0;

    if (M5.Touch.getCount() > 0) {
        auto touch = M5.Touch.getDetail();
        int y = touch.y;

        if (y >= 30 && y < 30 + 35 * 4) { // 4 rows
            int row = (y - 30) / 35;
            bool isLeftArrow = touch.x < 40;
            bool isRightArrow = touch.x > 280;

            if (isLeftArrow || isRightArrow) {
                unsigned long currentTime = millis();

                if (row != lastTouchedRow || isLeftArrow != lastTouchedLeft || currentTime - touchStartTime > 500) {
                    touchStartTime = currentTime;
                }

                if (currentTime - lastChangeTime >= CHANGE_DELAY) {
                    switch (row) {
                        case 0: // Current Baud Rate
                            currentBaudIndex = (currentBaudIndex + (isLeftArrow ? NUM_BAUDS - 1 : 1)) % NUM_BAUDS;
                            break;
                        case 1: // Target Servo ID
                            targetServoId = (targetServoId + (isLeftArrow ? 253 : 1)) % 254;
                            break;
                        case 2: // New Servo ID
                            newServoId = (newServoId + (isLeftArrow ? 253 : 1)) % 254;
                            if (newServoId == 0) newServoId = 1;
                            break;
                        case 3: // New Baud Rate
                            newBaudIndex = (newBaudIndex + (isLeftArrow ? NUM_BAUDS - 1 : 1)) % NUM_BAUDS;
                            break;
                    }
                    drawChangeIdBaudMode(BAUD_RATES[currentBaudIndex], targetServoId, newServoId, BAUD_RATES[newBaudIndex]);
                    lastChangeTime = currentTime;
                }

                lastTouchedRow = row;
                lastTouchedLeft = isLeftArrow;
            }
        } else if (y >= 182 && y < 212) { // Apply buttons
            // Apply buttons
            if (touch.x < 160) {
                // Apply New ID
                M5.Lcd.fillRect(10, 215, 310, 20, TFT_BLACK);
                M5.Lcd.setCursor(10, 215);
                M5.Lcd.print("Applying new ID...");

                // Set DYNAMIXEL to the current baud rate
                dxl.begin(BAUD_RATES[currentBaudIndex]);
                dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

                // Attempt to change the ID
                if (dxl.ping(targetServoId)) {
                    if (dxl.setID(targetServoId, newServoId)) {
                        M5.Lcd.fillRect(10, 215, 310, 20, TFT_BLACK);
                        M5.Lcd.setCursor(10, 215);
                        M5.Lcd.print("ID changed successfully!");
                        targetServoId = newServoId;  // Update the target ID
                    } else {
                        M5.Lcd.fillRect(10, 215, 310, 20, TFT_BLACK);
                        M5.Lcd.setCursor(10, 215);
                        M5.Lcd.print("Failed to change ID");
                    }
                } else {
                    M5.Lcd.fillRect(10, 215, 310, 20, TFT_BLACK);
                    M5.Lcd.setCursor(10, 215);
                    M5.Lcd.print("Servo not found");
                }
                delay(2000);  // Show the result for 2 seconds
                drawChangeIdBaudMode(BAUD_RATES[currentBaudIndex], targetServoId, newServoId, BAUD_RATES[newBaudIndex]);  // Redraw the screen
            } else {
                // Apply New Baud rate
                M5.Lcd.fillRect(10, 215, 310, 20, TFT_BLACK);
                M5.Lcd.setCursor(10, 215);
                M5.Lcd.print("Applying new Baud rate...");
                // Set DYNAMIXEL to the current baud rate
                dxl.begin(BAUD_RATES[currentBaudIndex]);
                dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

                // Attempt to change the Baud rate
                if (dxl.ping(targetServoId)) {
                    if (dxl.setBaudrate(targetServoId, BAUD_RATES[newBaudIndex])) {
                        M5.Lcd.fillRect(10, 215, 310, 20, TFT_BLACK);
                        M5.Lcd.setCursor(10, 215);
                        M5.Lcd.print("Baud changed successfully!");

                        // Update the current baud rate
                        currentBaudIndex = newBaudIndex;

                        // Reconnect with the new baud rate
                        delay(200);  // Wait for the servo to apply the new baud rate
                        dxl.begin(BAUD_RATES[currentBaudIndex]);
                    } else {
                        M5.Lcd.fillRect(10, 215, 310, 20, TFT_BLACK);
                        M5.Lcd.setCursor(10, 215);
                        M5.Lcd.print("Failed to change Baud rate");
                    }
                } else {
                    M5.Lcd.fillRect(10, 215, 310, 20, TFT_BLACK);
                    M5.Lcd.setCursor(10, 215);
                    M5.Lcd.print("Servo not found");
                }
                delay(2000);  // Show the result for 2 seconds
                drawChangeIdBaudMode(BAUD_RATES[currentBaudIndex], targetServoId, newServoId, BAUD_RATES[newBaudIndex]);  // Redraw the screen
            }
        }
    } else {
        lastTouchedRow = -1;
    }

    if (M5.BtnB.wasPressed()) {
        currentState = MAIN_MENU;
        drawMainMenu();
    }
}

// Hardware Error Status(70) bits (X series)
const uint8_t HW_ERR_INPUT_VOLTAGE    = 0x01;
const uint8_t HW_ERR_OVERHEATING      = 0x04;
const uint8_t HW_ERR_ENCODER          = 0x08;
const uint8_t HW_ERR_ELECTRICAL_SHOCK = 0x10;
const uint8_t HW_ERR_OVERLOAD         = 0x20;

// 期限付きでサーボの応答復帰を待つ
static bool waitServoReady(uint8_t id, uint32_t timeout_ms) {
    unsigned long start = millis();
    while (millis() - start < timeout_ms) {
        if (dxl.ping(id)) return true;
        delay(50);
    }
    return false;
}

// Position Modeサンプル: 現在位置から90度ずつ4ステップで1回転する。
// 通常のPosition Mode(3)は0-4095(1回転内)の絶対位置制御でラップアラウンドしないため、
// Extended Position Mode(4)の累積ゴールで行う。
// 失敗時はfalseを返し、errへ画面表示用メッセージを設定する。
static bool runPositionModeSample(uint8_t id, const char** err) {
    if (!dxl.ping(id)) {
        *err = "Servo not found";
        return false;
    }

    // Shutdown発火中(Torque落ち)はREBOOTでしか復帰できないが、
    // 原因を確認せず保護を解除しない: 電源サグ由来のInput Voltage/Overloadのみ復旧を試みる
    int32_t hwErr = dxl.readControlTableItem(ControlTableItem::HARDWARE_ERROR_STATUS, id);
    if (dxl.getLastLibErrCode() != DXL_LIB_OK) {
        *err = "HW status read failed";
        return false;
    }
    if (hwErr & (HW_ERR_OVERHEATING | HW_ERR_ENCODER | HW_ERR_ELECTRICAL_SHOCK)) {
        *err = "HW error: check servo!";
        return false;
    }
    if (hwErr & (HW_ERR_INPUT_VOLTAGE | HW_ERR_OVERLOAD)) {
        dxl.reboot(id);
        if (!waitServoReady(id, 2000)) {
            *err = "Reboot timeout";
            return false;
        }
    }

    if (!dxl.torqueOff(id)) {
        *err = "Torque off failed";
        return false;
    }
    if (!dxl.setOperatingMode(id, OP_EXTENDED_POSITION)) {
        *err = "Set mode failed";
        return false;
    }

    // Time-based ProfileだとProfile Velocity=100は「移動時間100ms」になるため、
    // Velocity-based Profile (Drive Mode bit2=0) を保証する (他ビットは保持、EEPROM書換は必要時のみ)
    int32_t driveMode = dxl.readControlTableItem(ControlTableItem::DRIVE_MODE, id);
    if (dxl.getLastLibErrCode() != DXL_LIB_OK) {
        *err = "Drive mode read failed";
        return false;
    }
    if ((driveMode & 0x04) &&
        !dxl.writeControlTableItem(ControlTableItem::DRIVE_MODE, id, driveMode & ~0x04)) {
        *err = "Set drive mode failed";
        return false;
    }

    // 最大速度スナップによる電流スパイク(電源サグ→Shutdown)を防ぐ
    if (!dxl.writeControlTableItem(ControlTableItem::PROFILE_ACCELERATION, id, 30) ||
        !dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, id, 100)) { // approx. 23 RPM
        *err = "Set profile failed";
        return false;
    }
    if (!dxl.torqueOn(id)) {
        *err = "Torque on failed";
        return false;
    }

    float startDeg = dxl.getPresentPosition(id, UNIT_DEGREE);
    if (dxl.getLastLibErrCode() != DXL_LIB_OK) {
        dxl.torqueOff(id);
        *err = "Position read failed";
        return false;
    }

    DEBUG_SERIAL.printf("[POS] model=%u driveMode=%ld startDeg=%.1f startRaw=%.0f\n",
                        dxl.getModelNumber(id), (long)driveMode, startDeg,
                        dxl.getPresentPosition(id, UNIT_RAW));

    for (int step = 1; step <= 4; step++) {
        float goalDeg = startDeg + 90.0f * step;
        if (!dxl.setGoalPosition(id, goalDeg, UNIT_DEGREE)) {
            dxl.torqueOff(id);
            *err = "Set goal failed";
            return false;
        }
        delay(1500);
        DEBUG_SERIAL.printf("[POS] step=%d goalDeg=%.1f presentDeg=%.1f presentRaw=%.0f\n",
                            step, goalDeg,
                            dxl.getPresentPosition(id, UNIT_DEGREE),
                            dxl.getPresentPosition(id, UNIT_RAW));
    }

    // 完走確認: 移動中のShutdown(Torque落ち)や未到達を成功扱いにしない
    hwErr = dxl.readControlTableItem(ControlTableItem::HARDWARE_ERROR_STATUS, id);
    bool stillTorqueOn = (dxl.readControlTableItem(ControlTableItem::TORQUE_ENABLE, id) == 1);
    float endDeg = dxl.getPresentPosition(id, UNIT_DEGREE);
    bool posReadOk = (dxl.getLastLibErrCode() == DXL_LIB_OK);
    dxl.torqueOff(id);

    if (hwErr != 0 || !stillTorqueOn) {
        *err = "HW shutdown during move";
        return false;
    }
    if (!posReadOk || fabsf(endDeg - (startDeg + 360.0f)) > 20.0f) {
        *err = "Goal not reached";
        return false;
    }
    return true;
}

// Handle Sample Execution Mode Touch Events
void handleSampleExecutionModeTouch() {
    static int lastTouchedRow = -1;
    static bool lastTouchedLeft = false;
    static unsigned long touchStartTime = 0;

    if (M5.Touch.getCount() > 0) {
        auto touch = M5.Touch.getDetail();
        int y = touch.y;

        if (y >= 30 && y < 30 + 35 * 2) { // サンプル実行モードの2つの行
            int row = (y - 30) / 35;
            bool isLeftArrow = touch.x < 40;
            bool isRightArrow = touch.x > 280;

            if (isLeftArrow || isRightArrow) {
                unsigned long currentTime = millis();

                if (row != lastTouchedRow || isLeftArrow != lastTouchedLeft || currentTime - touchStartTime > 500) {
                    touchStartTime = currentTime;
                }

                if (currentTime - lastChangeTime >= CHANGE_DELAY) {
                    switch (row) {
                        case 0: // Servo ID
                            sampleServoId = (sampleServoId + (isLeftArrow ?  -1 : 1)) % 254;
                            if (sampleServoId == 0) sampleServoId = 1;
                            break;
                        case 1: // Baud Rate
                            sampleBaudIndex = (sampleBaudIndex + (isLeftArrow ? NUM_BAUDS - 1 : 1)) % NUM_BAUDS;
                            break;
                    }
                    
                    drawSampleExecutionMode(BAUD_RATES[sampleBaudIndex], sampleServoId);
                    lastChangeTime = currentTime;
                }

                lastTouchedRow = row;
                lastTouchedLeft = isLeftArrow;
            }
        } else if (touch.wasPressed() // エッジトリガ: 押した瞬間のみ (レベルトリガだと1タップで複数回実行される)
                   && y >= BUTTON_Y && y < BUTTON_Y + BUTTON_HEIGHT
                   && ((touch.x >= 10 && touch.x < 10 + BUTTON_WIDTH)
                       || (touch.x >= 170 && touch.x < 170 + BUTTON_WIDTH))) { // Execute buttons
            bool isPositionBtn = (touch.x < 10 + BUTTON_WIDTH);

            // DYNAMIXELを選択されたボーレートに設定
            dxl.begin(BAUD_RATES[sampleBaudIndex]);
            dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

            bool sampleOk = false;
            const char* errMsg = "Execution failed!";

            if (isPositionBtn) {
                // Position Modeのサンプル実行
                M5.Lcd.fillRect(10, 220, 310, 20, TFT_BLACK); // "C: Back"行を消してステータス表示に使う (y=230だと下半分が見切れる)
                M5.Lcd.setCursor(10, 220);
                M5.Lcd.print("Running Position Mode...");

                sampleOk = runPositionModeSample(sampleServoId, &errMsg);
            } else {
                // Velocity Modeのサンプル実行
                M5.Lcd.fillRect(10, 220, 310, 20, TFT_BLACK); // "C: Back"行を消してステータス表示に使う (y=230だと下半分が見切れる)
                M5.Lcd.setCursor(10, 220);
                M5.Lcd.print("Running Velocity Mode...");

                // Velocity Mode Sample Code
                dxl.torqueOff(sampleServoId);
                dxl.setOperatingMode(sampleServoId, OP_VELOCITY);
                dxl.torqueOn(sampleServoId);

                // 30 RPMで回転
                dxl.setGoalVelocity(sampleServoId, 30, UNIT_RPM);
                delay(2000);

                // 反対方向に30 RPMで回転
                dxl.setGoalVelocity(sampleServoId, -30, UNIT_RPM);
                delay(2000);

                // 停止
                dxl.setGoalVelocity(sampleServoId, 0, UNIT_RPM);
                delay(1000);

                dxl.torqueOff(sampleServoId);
                sampleOk = true;
            }

            M5.Lcd.fillRect(10, 220, 310, 20, TFT_BLACK); // "C: Back"行を消してステータス表示に使う (y=230だと下半分が見切れる)
            M5.Lcd.setCursor(10, 220);
            M5.Lcd.print(sampleOk ? "Execution complete!" : errMsg);
            delay(2000);
            // 正しい変数を渡す
            drawSampleExecutionMode(BAUD_RATES[sampleBaudIndex], sampleServoId);
        }
    } else {
        lastTouchedRow = -1;
    }

    if (M5.BtnC.wasPressed()) {
        currentState = MAIN_MENU;
        drawMainMenu();
    }
}

// Handle Scan Mode
void handleScanMode() {
    if (M5.BtnA.wasPressed()) {
        currentState = MAIN_MENU;
        drawMainMenu();
    }
}

// Handle Change ID/Baudrate Mode
void handleChangeIdBaudMode() {
    handleChangeIdBaudModeTouch();
}

// Handle Sample Execution Mode
void handleSampleExecutionMode() {
    handleSampleExecutionModeTouch();
}

void setup() {
    M5.begin();
    delay(1000); // Wait for Startup DYNAMIXEL

    DEBUG_SERIAL.begin(115200);

    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextSize(2);

    DXL_SERIAL.begin(BAUD_RATES[0], SERIAL_8N1, PIN_RX_SERVO, PIN_TX_SERVO);

    M5.Lcd.setCursor(10, 10);
    M5.Lcd.println("Initializing...");

    for (int i = 0; i < NUM_BAUDS; i++) {
        dxl.begin(BAUD_RATES[i]);
        dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
        dxl.ping(DXL_BROADCAST_ID);
        delay(100);
    }

    drawMainMenu();
}

void loop() {
    M5.update();

    switch (currentState) {
        case MAIN_MENU:
            // Main Menuの操作処理
            if (M5.BtnA.wasPressed()) {
                currentState = SCAN_MODE;
                scanDynamixel();
            } else if (M5.BtnB.wasPressed()) {
                currentState = CHANGE_ID_BAUD_MODE;
                drawChangeIdBaudMode(BAUD_RATES[currentBaudIndex], targetServoId, newServoId, BAUD_RATES[newBaudIndex]);
            } else if (M5.BtnC.wasPressed()) {
                currentState = EXAMPLE_EXECUTION_MODE;
                drawSampleExecutionMode(BAUD_RATES[sampleBaudIndex], sampleServoId);
            }
            break;
        case SCAN_MODE:
            handleScanMode();
            break;
        case CHANGE_ID_BAUD_MODE:
            handleChangeIdBaudMode();
            break;
        case EXAMPLE_EXECUTION_MODE:
            handleSampleExecutionMode();
            break;
    }

    delay(10);
}
