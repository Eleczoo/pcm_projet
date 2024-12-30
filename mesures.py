# TEST PROGRAM
# ? Each number of city data file creates a different output file
# ? That contains:
# ? {TIME},{NB_THREADS},{LIMIT},{BEST_PATH}


import subprocess
import time
import os
import logging
import csv


MAX_LIMIT = 5
MAX_NB_CITIES = 8 # ! WE NEED AT LEAST THIS MUCH FILES WITH ALL THE CITIES

OUTPUT_DIR = "RESULTS"
if os.path.exists(OUTPUT_DIR):
	for file in os.listdir(OUTPUT_DIR):
		file_path = os.path.join(OUTPUT_DIR, file)
		if os.path.isfile(file_path):
			os.remove(file_path)
else:
	os.makedirs(OUTPUT_DIR)


# Linear space for number of threads, to avoid measring for hours
lin_space_th = [1, 6, 11, 16, 21, 27, 32, 37, 42, 47, 53, 58, 63, 68, 73, 79, 84, 89, 94, 99, 105, 110, 115, 120, 125, 131, 136, 141, 146, 151, 157, 162, 167, 172, 177, 183, 188, 193, 198, 203, 209, 214, 219, 224, 229, 235, 240, 245, 250, 256]


# Create logger file to log the output of the program, show the line number and the function name
if os.path.exists("mesures.log"):
	os.remove("mesures.log")
logging.basicConfig(filename="mesures.log", level=logging.INFO, format="%(asctime)s - %(levelname)s - %(funcName)s - %(lineno)d - %(message)s")

BEST_PATHS = [
	0,
	0,
	582, 
	1598,
	1602,
	1719,
	1818,
	1883,
	2101,
	2134,
	2577,
	3014,
	3026,
	3125,
	3161,
	3171,
	3226,
	3249,
	3270,
]


def main():
	for city_index in range(2, MAX_NB_CITIES+1):				
		tsp_file = f"./data/dj{city_index}.tsp"
		output_file = f"{OUTPUT_DIR}/dj{city_index}.csv"

		with open(output_file, "a", newline="") as csvfile:
			writer = csv.writer(csvfile)
			writer.writerow(["TIME", "NB_THREADS", "LIMIT", "BEST_PATH"])

		#for nb_threads in range(1, MAX_THREADS+1, 4):
		for nb_threads in lin_space_th:
			real_limit = city_index - MAX_LIMIT
			if real_limit < 0:
				real_limit = 0

			for limit in range(real_limit, city_index + 1):

				print(f"{tsp_file}\n- {nb_threads} Thread(s)\n- {limit} limit", end="\n\n")

				command = ["./build/main", tsp_file, str(nb_threads), str(limit)]

				with open(output_file, "a", newline="") as csvfile:
					writer = csv.writer(csvfile)
					try:
						start_time = time.perf_counter_ns()
						result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
						end_time = time.perf_counter_ns()

						elapsed_time = end_time - start_time
						elp_ms = elapsed_time / 1_000_000
						best_path = int(result.stdout[10:].split(":")[0])

						writer.writerow([f"{elp_ms:.06f}", nb_threads, limit, best_path])

						if best_path != BEST_PATHS[city_index]:
							logging.error(f"NOOOOOOOOOO - WRONG PATH {best_path} != {BEST_PATHS[city_index]}")

						logging.info(f"Command : {' '.join(command)}")
						logging.info(f"STDOUT : {result.stdout}")
						logging.info(f"TIME   : {elp_ms} ms")
						if result.stderr:
							logging.error(f"Standard Error:{result.stderr}")

					except Exception as e:
						logging.error(f"Error running command {' '.join(command)}:\n{str(e)}\n")

if __name__ == "__main__":
	main()
