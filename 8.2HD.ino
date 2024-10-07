const int buttonPin = 2;        // Button pin connected to the R16503 pushbutton
const int ledPins[] = {8, 7, 6, 5, 4, 3}; // Reverse LED pins
const int numLEDs = 6;          // Number of LEDs
int score = 0;                  // Player score
unsigned long startTime;        // Start time for game
unsigned long ledInterval;      // Interval for LED movement
bool canScore = false;          // Flag to control scoring

// Global variables for game state
static int currentLED = 0;      // Current LED position
static bool direction = true;    // Direction for LED movement

void setup() {
  Serial.begin(9600);
  while(!Serial);

  // Set LED pins as output
  for (int i = 0; i < numLEDs; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  // Set button pin as input with internal pull-up resistor
  pinMode(buttonPin, INPUT_PULLUP); // Button reads HIGH when not pressed

  Serial.println("System Successfully Initialized.");
  mainMenu(); // Start the main menu
}

void loop() {
  // Nothing to do in loop; all action happens in functions
}

// Main menu to select game mode
void mainMenu() {
  Serial.println("Select Your Game Mode:");
  Serial.println("0: Catch the Running Lights");
  Serial.println("1: Identify the LED");

  // Clear serial input buffer
  while (Serial.available() > 0) {
    Serial.read(); // Read and discard any available input
  }

  while (true) {
    if (Serial.available() > 0) {
      char choice = Serial.read();
      if (choice == '0') {
        Serial.println("Catch the Running Lights selected");
        configureGame();
        playGameMode0();
      } else if (choice == '1') {
        Serial.println("Identify the LED selected");
        configureGame();
        playGameMode1();
      } else {
        Serial.println("Invalid selection. Please enter (0 or 1):");
        delay(500); // Add a delay before checking again
      }
    }
  }
}

// Function to configure the game speed and difficulty settings
void configureGame() {
  Serial.println("0: Slow \n1: Medium \n2: Fast");

  // Clear serial input buffer
  while (Serial.available() > 0) {
    Serial.read(); // Read and discard any available input
  }

  while (true) {
    if (Serial.available() > 0) {
      char speedChoice = Serial.read();
      switch (speedChoice) {
        case '0': 
          ledInterval = 1000; 
          Serial.println("The speed has been set to Slow.");
          return; // Exit function after valid input
        case '1': 
          ledInterval = 500; 
          Serial.println("The speed has been set to Medium.");
          return; // Exit function after valid input
        case '2': 
          ledInterval = 250; 
          Serial.println("The speed has been set to Fast.");
          return; // Exit function after valid input
        default: 
          Serial.println("Invalid input. Please select (0, 1, or 2):");
          break; // Prompt again if input is invalid
      }
    }
  }
}

// Game Mode 0: Catch the Running Lights
void playGameMode0() {
  static unsigned long lastChangeTime = 0;
  static unsigned long lastButtonPressTime = 0; // New variable for button press time

  // Move the LED at intervals
  while (true) { // Use a while loop that will run until we break it
    if (score >= 20) { // Check if the game is over
      endGame(); // Call the endGame function
      return; // Exit the function
    }

    if (millis() - lastChangeTime >= ledInterval) {
      digitalWrite(ledPins[currentLED], LOW); // Turn off current LED

      // Update LED position
      currentLED = direction ? currentLED + 1 : currentLED - 1;

      // Reverse direction at the ends
      if (currentLED >= numLEDs - 1 || currentLED <= 0) {
        direction = !direction;
        canScore = true; // Allow scoring at the endpoints
      } else {
        canScore = false; // Disallow scoring when not at endpoints
      }

      digitalWrite(ledPins[currentLED], HIGH); // Turn on the new LED
      lastChangeTime = millis(); // Reset the interval timer
    }

    // Check for button press and scoring
    if (digitalRead(buttonPin) == LOW) { // Check for button press
      if (millis() - lastButtonPressTime > 200) { // 200 ms debounce delay
        lastButtonPressTime = millis(); // Update last button press time

        // Adjust score based on current LED position
        if (currentLED == 0 || currentLED == numLEDs - 1) {
          score += 2; // Correct press at the endpoint
        } else {
          score -= 1; // Incorrect press at other LEDs
        }

        Serial.print("Your current score: ");
        Serial.println(score);
      }
    }
  }
}

// Game Mode 1: Identify the LED
void playGameMode1() {
  static unsigned long lastButtonPressTime = 0; // For debouncing
  static bool buttonPressed = false; // Track the button state
  int targetLED; // The LED that will light up
  unsigned long targetDuration; // Duration for which the LED stays on

  // Randomly choose LED on duration
  while (score < 20) {
    targetLED = random(0, numLEDs); // Choose a random LED
    targetDuration = random(2000, 4000); // LED on for 2 to 4 seconds
    digitalWrite(ledPins[targetLED], HIGH); // Turn on the target LED
    delay(targetDuration); // Wait for the duration
    digitalWrite(ledPins[targetLED], LOW); // Turn off the target LED

    // Record the time limit for button presses
    unsigned long buttonPressDeadline = millis() + 1000; // 1 second to press the button
    int requiredPresses = targetLED + 1; // Number of required button presses

    // Allow the player to press the button the required number of times
    for (int i = 0; i < requiredPresses; i++) { // Expect requiredPresses
      bool pressed = false; // Track if the button is pressed

      while (millis() < buttonPressDeadline) { // Check if within the time limit
        int buttonState = digitalRead(buttonPin);

        // Check if the button is pressed and hasn't been counted yet
        if (buttonState == LOW && !buttonPressed) {
          if (millis() - lastButtonPressTime > 100) { // 100 ms debounce check
            lastButtonPressTime = millis(); // Update last button press time
            Serial.println("You pressed the button!"); // Debug statement
            Serial.print("Remaining required presses: ");
            Serial.println(requiredPresses - i); // Show how many more presses are needed

            // Increment score for correct presses
            score += 2; // Correct press
            Serial.print("Score now: ");
            Serial.println(score); // Show updated score
            pressed = true; // Mark that the button was pressed
            buttonPressed = true; // Set button pressed state
            break; // Exit the button press waiting loop
          }
        } else if (buttonState == HIGH) {
          buttonPressed = false; // Reset button pressed state when released
        }
      }

      // If the button was pressed, wait for it to be released before continuing
      if (pressed) {
        while (digitalRead(buttonPin) == LOW) {} // Wait until button is released
      }
    }

    // After the expected presses, check for any additional button presses
    unsigned long waitUntil = millis() + 1000; // Allow 1 second to register extra presses
    Serial.println("Waiting for extra button presses...");
    while (millis() < waitUntil) { // Wait until the deadline for extra presses
      if (digitalRead(buttonPin) == LOW) {
        if (millis() - lastButtonPressTime > 100) { // 100 ms debounce check
          lastButtonPressTime = millis(); // Update last button press time
          Serial.println("Extra button press detected after time limit."); // Debug statement
          score -= 1; // Miss press if button is pressed after time is up
          Serial.print("Score now adjusted to: ");
          Serial.println(score); // Show updated score
        }
      }
    }

    // Print the score after checking extra presses
    Serial.print("Final Score after checking extra presses: ");
    Serial.println(score); // Show final score after this round
  }

  endGame();
}

// Function to end the game and display the time taken
void endGame() {
  unsigned long endTime = millis();
  Serial.print("Game Finished! Duration was: ");
  Serial.print((endTime - startTime) / 1000);
  Serial.println(" seconds.");
  
  // Turn off all LEDs before ending the game
  for (int i = 0; i < numLEDs; i++) {
    digitalWrite(ledPins[i], LOW); // Turn off each LED
  }

  // Ask the player to start a new game or exit
  Serial.println("Press 'Y' to return to Main Menu or 'N' to Exit:");

  // Clear serial input buffer
  while (Serial.available() > 0) {
    Serial.read(); // Read and discard any available input
  }

  // Wait for user input
  while (true) {
    if (Serial.available() > 0) {
      char choice = Serial.read();
      if (choice == 'Y' || choice == 'y') {
        resetGameState(); // Reset game state before returning to the main menu
        mainMenu(); // Go back to main menu
      } else if (choice == 'N' || choice == 'n') {
        Serial.println("Exiting game. Thank you!");
        while (true); // Infinite loop to exit the program
      } else {
        Serial.println("Invalid input. Please press 'Y' or 'N':");
      }
    }
  }
}

// Function to reset game state
void resetGameState() {
  score = 0; // Reset score
  currentLED = 0; // Reset LED position
  direction = true; // Reset direction for LED movement
  canScore = false; // Reset scoring flag
  startTime = millis(); // Reset the start time for the new game
}
