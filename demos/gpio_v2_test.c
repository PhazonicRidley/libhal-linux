#include <fcntl.h>
#include <linux/gpio.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main()
{
  struct gpio_v2_line_request output_req;
  struct gpio_v2_line_values output_values;
  struct gpio_v2_line_request input_req;
  struct gpio_v2_line_values input_values;
  int chip_fd = open("/dev/gpiochip0", O_RDONLY);
  if (chip_fd < 0) {
    perror("gpiochip0");
    return 1;
  }
  memset(&output_req, 0, sizeof(struct gpio_v2_line_request));
  memset(&output_values, 0, sizeof(struct gpio_v2_line_values));
  memset(&input_req, 0, sizeof(struct gpio_v2_line_request));
  memset(&input_values, 0, sizeof(struct gpio_v2_line_values));

  output_req.offsets[0] = 2;                           // GPIO number
  output_req.num_lines = 1;                            // number of GPIOs
  output_req.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;  // set as output

  input_req.offsets[0] = 3;
  input_req.num_lines = 1;
  input_req.config.flags = GPIO_V2_LINE_FLAG_INPUT;

  if (ioctl(chip_fd, GPIO_V2_GET_LINE_IOCTL, &output_req) < 0) {
    perror("getting output line");
    close(chip_fd);
    return 1;
  }

  if (ioctl(chip_fd, GPIO_V2_GET_LINE_IOCTL, &input_req) < 0) {
    perror("getting input line");
    close(chip_fd);
    return 1;
  }

  int output_req_fd = output_req.fd;
  int input_req_fd = input_req.fd;
  output_values.mask = 1;
  output_values.bits = 1;
  input_values.mask = 1;

  struct gpio_v2_line_values recv;
  memset(&recv, 0, sizeof(struct gpio_v2_line_values));
  recv.mask = 1;

  for (int i = 0; i < 25; i++) {
    if (ioctl(output_req_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &output_values) <
        0) {
      perror("setting value");
      return 1;
    }
    if (ioctl(output_req_fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &recv) < 0) {
      perror("getting output value");
      return 1;
    }
    if (ioctl(input_req_fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &input_values) < 0) {
      perror("getting input value");
      return 1;
    }
    output_values.bits = ~(output_values.bits) & output_values.mask;
    printf("Got output: %llu\n", recv.bits);
    printf("Got input: %llu\n", input_values.bits);
    sleep(2);
  }
  close(output_req_fd);
  close(chip_fd);
  return 0;
}