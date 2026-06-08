void SPI_MasterInit(void);

unsigned char SPI_MasterTransmit(unsigned char cData);

void SPI_SlaveInit(void);

char SPI_SlaveReceive(char data);
int ReadTemperature(void);
