import os
import argparse
import time

def main():

    parser = argparse.ArgumentParser(description="Calculate prime numbers up to a maximum value")

    parser.add_argument("--max-prime", type=int, required=True, help="The maximum prime number to calculate")
    parser.add_argument("--worker-count", type=int, default=4, help="The number of worker processes to spawn")

    # build the c code
    os.system("make")

    args = parser.parse_args()

    # start timer
    start_time = time.time()

    # run the boss process
    os.system(f"./build/boss {args.worker_count} {args.max_prime}")

    end_time = time.time()
    time_taken = end_time - start_time

    # calculate the amount of results
    result_count = 0
    with open("primes.txt", "r") as f:
        lines = f.readlines()
        result_count = len(lines)

    print("Cleaning up...")
    os.system("rm -r data")

    print("Outputed results to primes.txt")
    print(f"Found {result_count} prime numbers in {time_taken:.2f} seconds")
    print(f"Avg: {result_count / time_taken:.2f} primes/second")
    print("Done.")

if __name__ == "__main__":
    main()