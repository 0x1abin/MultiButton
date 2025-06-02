/*
 * MultiButton Library Advanced Example
 * This example demonstrates advanced features and dynamic button management
 */

#include "multi_button.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUTTONS 4

// Button instances
static Button buttons[MAX_BUTTONS];
static volatile int running = 1;
static int button_states[MAX_BUTTONS] = {0};

// Configuration
static int demo_mode = 1;
static int verbose_mode = 0;

// Signal handler for graceful exit
void signal_handler(int sig)
{
    if (sig == SIGINT) {
        printf("\n🛑 Received SIGINT, cleaning up...\n");
        running = 0;
    }
}

// Hardware abstraction layer function
uint8_t read_button_gpio(uint8_t button_id)
{
    if (button_id > 0 && button_id <= MAX_BUTTONS) {
        return button_states[button_id - 1];
    }
    return 0;
}

// Generic event handler that shows button info
void generic_event_handler(Button* btn, const char* event_name)
{
    if (verbose_mode) {
        printf("🔘 Button %d: %s (repeat: %d, pressed: %s)\n", 
               btn->button_id, 
               event_name,
               button_get_repeat_count(btn),
               button_is_pressed(btn) ? "Yes" : "No");
    } else {
        printf("🔘 Button %d: %s\n", btn->button_id, event_name);
    }
}

// Event handlers
void on_press_down(Button* btn) { generic_event_handler(btn, "Press Down"); }
void on_press_up(Button* btn) { generic_event_handler(btn, "Press Up"); }
void on_single_click(Button* btn) { generic_event_handler(btn, "Single Click"); }
void on_double_click(Button* btn) { generic_event_handler(btn, "Double Click"); }
void on_long_press_start(Button* btn) { generic_event_handler(btn, "Long Press Start"); }
void on_long_press_hold(Button* btn) { generic_event_handler(btn, "Long Press Hold"); }
void on_press_repeat(Button* btn) { generic_event_handler(btn, "Press Repeat"); }

// Special handler for button configuration
void on_config_button_click(Button* btn)
{
    static int config_state = 0;
    
    printf("⚙️ Config Button %d clicked!\n", btn->button_id);
    
    switch (config_state) {
        case 0:
            verbose_mode = !verbose_mode;
            printf("📝 Verbose mode: %s\n", verbose_mode ? "ON" : "OFF");
            break;
        case 1:
            demo_mode = !demo_mode;
            printf("🎭 Demo mode: %s\n", demo_mode ? "ON" : "OFF");
            break;
        case 2:
            printf("🔄 Resetting all buttons...\n");
            for (int i = 0; i < MAX_BUTTONS; i++) {
                button_reset(&buttons[i]);
            }
            break;
        case 3:
            printf("👋 Stopping demo...\n");
            running = 0;
            break;
    }
    
    config_state = (config_state + 1) % 4;
}

// Initialize a single button with all event handlers
void init_button(int index, uint8_t button_id, int enable_all_events)
{
    button_init(&buttons[index], read_button_gpio, 1, button_id);
    
    if (enable_all_events) {
        button_attach(&buttons[index], BTN_PRESS_DOWN, on_press_down);
        button_attach(&buttons[index], BTN_PRESS_UP, on_press_up);
        button_attach(&buttons[index], BTN_SINGLE_CLICK, on_single_click);
        button_attach(&buttons[index], BTN_DOUBLE_CLICK, on_double_click);
        button_attach(&buttons[index], BTN_LONG_PRESS_START, on_long_press_start);
        button_attach(&buttons[index], BTN_LONG_PRESS_HOLD, on_long_press_hold);
        button_attach(&buttons[index], BTN_PRESS_REPEAT, on_press_repeat);
    } else {
        // Only essential events
        button_attach(&buttons[index], BTN_SINGLE_CLICK, on_single_click);
        button_attach(&buttons[index], BTN_DOUBLE_CLICK, on_double_click);
        button_attach(&buttons[index], BTN_LONG_PRESS_START, on_long_press_start);
    }
    
    button_start(&buttons[index]);
}

// Initialize all buttons
void buttons_init(void)
{
    printf("🔧 Initializing %d buttons...\n", MAX_BUTTONS);
    
    // Button 1: Full feature set
    init_button(0, 1, 1);
    printf("  ✅ Button 1: Full feature set\n");
    
    // Button 2: Essential events only
    init_button(1, 2, 0);
    printf("  ✅ Button 2: Essential events only\n");
    
    // Button 3: Configuration button with special handler
    init_button(2, 3, 0);
    button_detach(&buttons[2], BTN_SINGLE_CLICK);
    button_attach(&buttons[2], BTN_SINGLE_CLICK, on_config_button_click);
    printf("  ✅ Button 3: Configuration button\n");
    
    // Button 4: Dynamic configuration demo
    init_button(3, 4, 0);
    printf("  ✅ Button 4: Dynamic configuration demo\n");
    
    printf("🎯 All buttons initialized successfully!\n\n");
}

// Simulate button press
void simulate_button_press(int button_id, int duration_ms)
{
    if (button_id < 1 || button_id > MAX_BUTTONS) return;
    
    if (verbose_mode) {
        printf("📱 Simulating button %d press (%d ms)\n", button_id, duration_ms);
    }
    
    button_states[button_id - 1] = 1;
    
    for (int i = 0; i < duration_ms / 5; i++) {
        button_ticks();
        usleep(5000);
    }
    
    button_states[button_id - 1] = 0;
    
    for (int i = 0; i < 10; i++) {
        button_ticks();
        usleep(5000);
    }
}

// Dynamic configuration demo
void dynamic_config_demo(void)
{
    printf("\n🔄 Dynamic Configuration Demo\n");
    printf("=====================================\n");
    
    // Initially button 4 has minimal handlers
    printf("1. Testing button 4 with minimal handlers...\n");
    simulate_button_press(4, 100);
    
    usleep(300000);
    
    // Add more handlers dynamically
    printf("2. Adding more event handlers to button 4...\n");
    button_attach(&buttons[3], BTN_PRESS_DOWN, on_press_down);
    button_attach(&buttons[3], BTN_PRESS_UP, on_press_up);
    button_attach(&buttons[3], BTN_PRESS_REPEAT, on_press_repeat);
    
    printf("3. Testing button 4 with full handlers...\n");
    simulate_button_press(4, 100);
    
    usleep(300000);
    
    // Remove some handlers
    printf("4. Removing press down/up handlers...\n");
    button_detach(&buttons[3], BTN_PRESS_DOWN);
    button_detach(&buttons[3], BTN_PRESS_UP);
    
    printf("5. Testing button 4 with reduced handlers...\n");
    simulate_button_press(4, 100);
}

// Interactive demo sequence
void run_demo_sequence(void)
{
    printf("\n🎭 Interactive Demo Sequence\n");
    printf("=====================================\n");
    
    printf("Demo 1: Single clicks on all buttons\n");
    for (int i = 1; i <= MAX_BUTTONS; i++) {
        simulate_button_press(i, 100);
        usleep(200000);
    }
    
    printf("\nDemo 2: Double click patterns\n");
    simulate_button_press(1, 80);
    usleep(50000);
    simulate_button_press(1, 80);
    usleep(500000);
    
    printf("\nDemo 3: Long press demonstration\n");
    simulate_button_press(2, 1200);
    usleep(300000);
    
    printf("\nDemo 4: Rapid press sequence\n");
    for (int i = 0; i < 4; i++) {
        simulate_button_press(1, 60);
        usleep(70000);
    }
    usleep(500000);
    
    printf("\nDemo 5: Configuration button test\n");
    for (int i = 0; i < 3; i++) {
        simulate_button_press(3, 100);
        usleep(200000);
    }
}

// Print button status
void print_button_status(void)
{
    printf("\n📊 Button Status Report\n");
    printf("========================\n");
    for (int i = 0; i < MAX_BUTTONS; i++) {
        printf("Button %d: ", buttons[i].button_id);
        printf("State=%d, ", button_is_pressed(&buttons[i]));
        printf("Repeat=%d, ", button_get_repeat_count(&buttons[i]));
        ButtonEvent event = button_get_event(&buttons[i]);
        printf("Event=%d\n", event);
    }
}

// Main function
int main(int argc, char* argv[])
{
    printf("🚀 MultiButton Library Advanced Example\n");
    printf("==========================================\n");
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose_mode = 1;
        } else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
            demo_mode = 0;
        }
    }
    
    printf("Configuration: Demo=%s, Verbose=%s\n\n", 
           demo_mode ? "ON" : "OFF", 
           verbose_mode ? "ON" : "OFF");
    
    // Set up signal handler
    signal(SIGINT, signal_handler);
    
    // Initialize buttons
    buttons_init();
    
    if (demo_mode) {
        // Run demonstration sequence
        run_demo_sequence();
        
        // Dynamic configuration demo
        dynamic_config_demo();
        
        // Print final status
        print_button_status();
        
        printf("\n✅ Advanced demo completed!\n");
        printf("💡 Use Ctrl+C to exit, or run with --quiet for manual testing\n");
    } else {
        printf("🎮 Manual test mode - buttons are ready for interaction\n");
        printf("💡 Use Ctrl+C to exit\n");
    }
    
    // Keep running until interrupted
    while (running) {
        button_ticks();
        usleep(5000); // 5ms tick
    }
    
    // Cleanup
    printf("\n🧹 Cleaning up...\n");
    for (int i = 0; i < MAX_BUTTONS; i++) {
        button_stop(&buttons[i]);
    }
    
    printf("👋 Advanced example finished!\n");
    return 0;
}

/*
 * Build and run instructions:
 * 
 * Build:
 * make advanced_example
 * 
 * Run with demo:
 * ./build/bin/advanced_example
 * 
 * Run with verbose output:
 * ./build/bin/advanced_example -v
 * 
 * Run in quiet mode (manual testing):
 * ./build/bin/advanced_example -q
 */ 