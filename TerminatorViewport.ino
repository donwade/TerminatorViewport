#include <Arduino.h>

#include "mRangeVelocity.h"
#include <M5Unified.h>
#include <M5StackMenuSystem.h>
#include "addin-ota.h"

Menu mainMenu("Main Menu");
Menu subMenu("Sub Menu");

void setup() {

	M5.begin();
	M5.Power.begin();
	Serial.begin(115200);
	while (!Serial) delay(1000);
	
    setup_c4001();
	setup_ota();
	
	mainMenu.addMenuItem("One Time Callback", callAndReturnUnattended);
	mainMenu.addMenuItem("Loop Callback", initESCkeywithCB, showTickUntilESC);
	mainMenu.addSubMenu("Submenu", &subMenu);
	mainMenu.addExitItem("Exit");

	subMenu.addMenuItem("Sub Menu Item #1", callAndReturnUnattended);
	subMenu.addMenuItem("Sub Menu Item #2", callAndReturnUnattended);
	subMenu.addMenuItem("Sub Menu Item #3", callAndReturnUnattended);
	subMenu.addMenuItem("Sub Menu Item #4", callAndReturnUnattended);
	subMenu.addMenuItem("Sub Menu Item #5", callAndReturnUnattended);
	subMenu.addMenuItem("Sub Menu Item #6", callAndReturnUnattended);
}

void loop() {
	M5.update();

    loop_c4001(); 
	loop_ota();
	
	if (mainMenu.isEnabled()) {
		mainMenu.loop();
	} else {
		M5.Display.clear(BLACK);
		if (M5.BtnA.wasReleased() || M5.BtnB.wasReleased() || M5.BtnC.wasReleased() || M5.Touch.getCount() > 0) {
			M5.update();
			mainMenu.enable();
		}
	}
}

void callAndReturnUnattended(CallbackMenuItem& menuItem) {
	M5.Display.clear(BLACK);
	drawText("One Time Callback");
	delay(1500);
}

void initESCkeywithCB(CallbackMenuItem& menuItem) {
	M5.Display.clear(BLACK);
	drawText("Loop Callback");
	menuItem.getMenu()->displaySoftKey(BtnASlot, "Esc");
}

void showTickUntilESC(CallbackMenuItem& menuItem) {
	String time = "sec: " + String(millis() / 1000);
	drawText(time);

	if (menuItem.getMenu()->wasSoftKeyReleased(BtnASlot))
	{
		menuItem.deactivateCallbacks();
	}
}

void drawText(String text)
{
	M5.Display.setTextColor(WHITE, BLACK);
	M5.Display.setTextFont(2);
	M5.Display.setTextSize(3);
	M5.Display.setTextDatum(MC_DATUM);
	M5.Display.drawString(text, 
						  M5.Display.width() / 2,
						  (3 * M5.Display.height()) / 6);
}

