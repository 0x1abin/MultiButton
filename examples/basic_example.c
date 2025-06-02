/*
 * MultiButton Library Basic Example
 * This example demonstrates basic usage of the optimized MultiButton library
 */

#include "multi_button.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

// Button instances
static Button btn1, btn2;
static volatile int running = 1;

// Simulate GPIO state for demonstration
static int btn1_state = 0;
static int btn2_state = 0;

// Signal handler for graceful exit
void signal_handler(int sig)
{
    if (sig == SIGINT) {
        printf("\nReceived SIGINT, exiting...\n");
        running = 0;
    }
}

// Hardware abstraction layer function
// This simulates reading GPIO states
uint8_t read_button_gpio(uint8_t button_id)
{
    switch (button_id) {
        case 1:
            return btn1_state;
        case 2:
            return btn2_state;
        default:
            return 0;
    }
}

// Callback functions for button 1
void btn1_single_click_handler(Button* btn)
{
    (void)btn;  // suppress unused parameter warning
    printf("🔘 Button 1: Single Click\n");
}

void btn1_double_click_handler(Button* btn)
{
    (void)btn;  // suppress unused parameter warning
    printf("🔘🔘 Button 1: Double Click\n");
}

void btn1_long_press_start_handler(Button* btn)
{
    (void)btn;  // suppress unused parameter warning
    printf("⏹️ Button 1: Long Press Start\n");
}

void btn1_long_press_hold_handler(Button* btn)
{
    (void)btn;  // suppress unused parameter warning
    printf("⏸️ Button 1: Long Press Hold...\n");
}

void btn1_press_repeat_handler(Button* btn)
{
    printf("🔄 Button 1: Press Repeat (count: %d)\n", button_get_repeat_count(btn));
}

// Callback functions for button 2
void btn2_single_click_handler(Button* btn)
{
    (void)btn;  // suppress unused parameter warning
    printf("🔵 Button 2: Single Click\n");
}

void btn2_double_click_handler(Button* btn)
{
    (void)btn;  // suppress unused parameter warning
    printf("🔵🔵 Button 2: Double Click\n");
}

void btn2_press_down_handler(Button* btn)
{
    (void)btn;  // suppress unused parameter warning
    printf("⬇️ Button 2: Press Down\n");
}

void btn2_press_up_handler(Button* btn)
{
    (void)btn;  // suppress unused parameter warning
    printf("⬆️ Button 2: Press Up\n");
}

// Initialize buttons
void buttons_init(void)
{
    // Initialize button 1 (active high for simulation)
    button_init(&btn1, read_button_gpio, 1, 1);
    
    // Attach event handlers for button 1
    button_attach(&btn1, BTN_SINGLE_CLICK, btn1_single_click_handler);
    button_attach(&btn1, BTN_DOUBLE_CLICK, btn1_double_click_handler);
    button_attach(&btn1, BTN_LONG_PRESS_START, btn1_long_press_start_handler);
    button_attach(&btn1, BTN_LONG_PRESS_HOLD, btn1_long_press_hold_handler);
    button_attach(&btn1, BTN_PRESS_REPEAT, btn1_press_repeat_handler);
    
    // Initialize button 2 (active high for simulation)
    button_init(&btn2, read_button_gpio, 1, 2);
    
    // Attach event handlers for button 2
    button_attach(&btn2, BTN_SINGLE_CLICK, btn2_single_click_handler);
    button_attach(&btn2, BTN_DOUBLE_CLICK, btn2_double_click_handler);
    button_attach(&btn2, BTN_PRESS_DOWN, btn2_press_down_handler);
    button_attach(&btn2, BTN_PRESS_UP, btn2_press_up_handler);
    
    // Start button processing
    button_start(&btn1);
    button_start(&btn2);
}

// Simulate button press for demonstration
void simulate_button_press(int button_id, int duration_ms)
{
    printf("\n📱 Simulating button %d press for %d ms...\n", button_id, duration_ms);
    
    if (button_id == 1) {
        btn1_state = 1;
    } else if (button_id == 2) {
        btn2_state = 1;
    }
    
    // Let the button library process the press
    for (int i = 0; i < duration_ms / 5; i++) {
        button_ticks();
        usleep(5000); // 5ms delay
    }
    
    // Release the button
    if (button_id == 1) {
        btn1_state = 0;
    } else if (button_id == 2) {
        btn2_state = 0;
    }
    
    // Process the release
    for (int i = 0; i < 10; i++) {
        button_ticks();
        usleep(5000); // 5ms delay
    }
}

// Main function
int main(void)
{
    printf("🚀 MultiButton Library Basic Example\n");
    printf("=====================================\n\n");
    
    // Set up signal handler
    signal(SIGINT, signal_handler);
    
    // Initialize buttons
    buttons_init();
    printf("✅ Buttons initialized successfully\n\n");
    
    printf("📋 Demo sequence:\n");
    printf("1. Single click simulation\n");
    printf("2. Double click simulation\n");
    printf("3. Long press simulation\n");
    printf("4. Repeat press simulation\n\n");
    
    // Demo sequence
    printf("--- Single Click Demo ---\n");
    simulate_button_press(1, 100);  // Short press
    usleep(500000); // Wait 500ms
    
    printf("\n--- Double Click Demo ---\n");
    simulate_button_press(1, 100);  // First click
    usleep(50000);  // Quick gap
    simulate_button_press(1, 100);  // Second click
    usleep(500000); // Wait for timeout
    
    printf("\n--- Long Press Demo ---\n");
    simulate_button_press(1, 1500); // Long press
    usleep(200000); // Wait
    
    printf("\n--- Repeat Press Demo ---\n");
    for (int i = 0; i < 3; i++) {
        simulate_button_press(2, 80);
        usleep(80000); // Quick succession
    }
    usleep(500000); // Wait for timeout
    
    printf("\n--- Button State Query Demo ---\n");
    printf("Button 1 pressed: %s\n", button_is_pressed(&btn1) ? "Yes" : "No");
    printf("Button 2 pressed: %s\n", button_is_pressed(&btn2) ? "Yes" : "No");
    printf("Button 1 repeat count: %d\n", button_get_repeat_count(&btn1));
    printf("Button 2 repeat count: %d\n", button_get_repeat_count(&btn2));
    
    printf("\n✅ Demo completed successfully!\n");
    printf("💡 In a real application, button_ticks() would be called from a 5ms timer interrupt.\n");
    
    return 0;
}

/*
 * Build instructions:
 * 
 * From project root directory:
 * make basic_example
 * 
 * Or build all examples:
 * make examples
 * 
 * Run the example:
 * ./build/bin/basic_example
 */ 