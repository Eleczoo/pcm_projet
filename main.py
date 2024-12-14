



def show_duplicate_lines(file_path):
	# Open the file in read mode
	with open(file_path, 'r') as file:
		# Read the file and split it into lines
		lines = file.readlines()
		# Create a set to store the lines
		lines_set = set()
		# Create a set to store the duplicate lines
		duplicate_lines = []
		# Iterate through the lines
		for line in lines:
			# If the line is already in the set, add it to the duplicate lines set
			if line in lines_set:
				duplicate_lines.append(line)
			# Otherwise, add it to the set
			else:
				lines_set.add(line)
	# Return the duplicate lines
	return duplicate_lines

def main():
	# Get the file path from the user
	file_path = "file.txt"
	# Get the duplicate lines
	duplicate_lines = show_duplicate_lines(file_path)
	# Print the duplicate lines
	for line in duplicate_lines:
		print(line.strip())


if __name__ == '__main__':
	main()
