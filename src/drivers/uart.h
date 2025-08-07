#define UART_TX_PIN 8 // UART PIN FOR BLUETOOTH
#define UART_RX_PIN 9 // UART PIN FOR BLUETOOTH
#define UART_ID uart1 // UART ID FOR BLUETOOTH

#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// Declare external variables for use in the interrupt handler.
extern volatile bool input_ready;      
extern volatile unsigned int ind;      
extern volatile char buffer[100];
extern char direction;
extern char prev_command;


void on_uart_rx(void);

void initialize_uart();

void reset_buffer();

void movement_command();

void initialize_bluetooth();

void prev_movement_command();

void send_accel_readings();
