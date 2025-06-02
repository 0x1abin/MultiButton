/*
 * MultiButton Library Polling Example
 * This example demonstrates polling-based button event detection
 */

#include "multi_button.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

// Button instance
static Button btn1;
static volatile int running = 1;

// Signal handler for graceful exit
void signal_handler(int sig)
{
    if (sig == SIGINT) {
        printf("\n🛑 Exiting polling example...\n");
        running = 0;
    }
}

// Hardware abstraction layer function
uint8_t read_button_gpio(uint8_t button_id)
{
    (void)button_id;  // suppress unused parameter warning
    
    // Simulate button state changes for demonstration
    static int cycle_count = 0;
    static int pattern_index = 0;
    
    // Predefined button press patterns for demonstration
    // Each number represents the state for several cycles
    int patterns[] = {
        0, 0, 0, 0, 0,    // Idle
        1, 1, 1, 1, 0,    // Short press
        0, 0, 0, 0, 0,    // Idle
        1, 1, 0, 1, 1,    // Double click
        0, 0, 0, 0, 0,    // Idle
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,  // Long press
        0, 0, 0, 0, 0,    // Idle
        1, 0, 1, 0, 1, 0, 1, 0,  // Rapid clicks
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // Final idle
    };
    
    int pattern_size = sizeof(patterns) / sizeof(patterns[0]);
    
    if (cycle_count++ >= 10) {  // Change pattern every 50ms (10 * 5ms)
        cycle_count = 0;
        pattern_index = (pattern_index + 1) % pattern_size;
    }
    
    return patterns[pattern_index];
}

// Initialize button without callbacks (polling mode)
void button_init_polling(void)
{
    printf("🔧 Initializing button for polling mode...\n");
    
    // Initialize button but don't attach any callbacks
    button_init(&btn1, read_button_gpio, 1, 1);
    
    // Start button processing
    button_start(&btn1);
    
    printf("✅ Button initialized for polling\n\n");
}

// Poll button events and handle them
void poll_and_handle_events(void)
{
    static ButtonEvent last_event = BTN_NONE_PRESS;
    static int event_count = 0;
    
    // Get current event
    ButtonEvent current_event = button_get_event(&btn1);
    
    // Only process if event changed
    if (current_event != last_event && current_event != BTN_NONE_PRESS) {
        event_count++;
        
        printf("📡 [%d] Polled Event: ", event_count);
        
        switch (current_event) {
            case BTN_PRESS_DOWN:
                printf("Press Down");
                break;
            case BTN_PRESS_UP:
                printf("Press Up");
                break;
            case BTN_SINGLE_CLICK:
                printf("Single Click ✨");
                break;
            case BTN_DOUBLE_CLICK:
                printf("Double Click ✨✨");
                break;
            case BTN_LONG_PRESS_START:
                printf("Long Press Start 🔥");
                break;
            case BTN_LONG_PRESS_HOLD:
                printf("Long Press Hold 🔥🔥");
                break;
            case BTN_PRESS_REPEAT:
                printf("Press Repeat (count: %d) 🔄", button_get_repeat_count(&btn1));
                break;
            default:
                printf("Unknown Event ❓");
                break;
        }
        
        printf(" | Pressed: %s", button_is_pressed(&btn1) ? "Yes" : "No");
        printf("\n");
        
        last_event = current_event;
    }
}

// Print periodic status
void print_status(void)
{
    static int status_counter = 0;
    
    if (++status_counter >= 200) {  // Print every 1 second (200 * 5ms)
        status_counter = 0;
        
        printf("📊 Status - Pressed: %s, Repeat: %d, Event: %d\n",
               button_is_pressed(&btn1) ? "Yes" : "No",
               button_get_repeat_count(&btn1),
               button_get_event(&btn1));
    }
}

// Main function
int main(void)
{
    printf("🚀 MultiButton Library Polling Example\n");
    printf("========================================\n\n");
    
    printf("💡 This example demonstrates polling-based event detection\n");
    printf("📡 Events are detected by polling button_get_event() instead of using callbacks\n");
    printf("🎬 A predefined pattern will simulate button presses\n\n");
    
    // Set up signal handler
    signal(SIGINT, signal_handler);
    
    // Initialize button for polling
    button_init_polling();
    
    printf("🎭 Starting simulation with predefined patterns...\n");
    printf("   Pattern includes: short press, double click, long press, rapid clicks\n");
    printf("   Press Ctrl+C to exit\n\n");
    
    int tick_count = 0;
    
    // Main polling loop
    while (running) {
        // Update button state machine
        button_ticks();
        
        // Poll for events
        poll_and_handle_events();
        
        // Print periodic status
        print_status();
        
        // Simulate 5ms tick interval
        usleep(5000);
        
        // Stop after reasonable demo time
        if (++tick_count > 2000) {  // 10 seconds
            printf("\n🏁 Demo pattern completed!\n");
            break;
        }
    }
    
    // Cleanup
    printf("\n🧹 Cleaning up...\n");
    button_stop(&btn1);
    
    printf("✅ Polling example finished!\n");
    printf("\n📚 Key takeaways:\n");
    printf("   • Polling mode allows checking events at your own pace\n");
    printf("   • No callback functions needed\n");
    printf("   • Use button_get_event() to check current event\n");
    printf("   • Still need to call button_ticks() every 5ms\n");
    printf("   • Useful for main loop architectures without interrupts\n");
    
    return 0;
}

/*
 * Build and run instructions:
 * 
 * Build:
 * make poll_example
 * 
 * Run:
 * ./build/bin/poll_example
 * 
 * This example shows:
 * - How to use the library without callbacks
 * - Polling-based event detection
 * - Integration with main loop architecture
 * - Status monitoring and reporting
 */ 