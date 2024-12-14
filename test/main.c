#include <iostream>

int main()
{
	// Allocate a buffer using `new` to store 100 integers
	int* buffer = new int[100];

	// Fill the buffer with values from 0 to 99
	for (int i = 0; i < 100; ++i)
	{
		buffer[i] = i;
	}

	// Print the values to verify
	std::cout << "Buffer values:\n";
	for (int i = 0; i < 100; ++i)
	{
		std::cout << buffer[i] << " ";
		if ((i + 1) % 10 == 0)
		{ // Print a newline every 10 values for readability
			std::cout << "\n";
		}
	}

	// Free the allocated memory
	delete[] buffer;

	return 0;
}
