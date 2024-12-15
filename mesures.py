import subprocess
import time

def run_tspcc():
	# Nom du fichier de sortie
	output_file = "results.txt"

	# Ouvrir le fichier en mode écriture (écrase le fichier s'il existe déjà)
	with open(output_file, "w") as f:
		f.write("Results for ./tspcc executions:\n")

	# Boucle à travers les fichiers dj1.tsp à dj17.tsp
	for i in range(1, 18):
		tsp_file = f"./data/dj{i}.tsp"
		command = ["./build/main", tsp_file]

		try:
			# Mesure du temps d'exécution
			start_time = time.time()
			result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
			end_time = time.time()

			# Calcul du temps écoulé
			elapsed_time = end_time - start_time

			# Enregistrement des sorties dans le fichier
			with open(output_file, "a") as f:
				f.write(f"\nCommand: {' '.join(command)}\n")
				f.write(f"Standard Output:\n{result.stdout}\n")
				f.write(f"Standard Error:\n{result.stderr}\n")
				f.write(f"Elapsed Time: {elapsed_time:.6f} seconds\n")
				f.write("-" * 40 + "\n")

		except Exception as e:
			# En cas d'erreur, enregistrer l'exception dans le fichier
			with open(output_file, "a") as f:
				f.write(f"Error running command {' '.join(command)}:\n{str(e)}\n")

if __name__ == "__main__":
	run_tspcc()
