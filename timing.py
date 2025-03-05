import sys
from collections import defaultdict

def count_lines():
    """
    Reads lines from standard input and counts their occurrences.
    Returns a dictionary with lines as keys and their counts as values.
    """
    line_counts = defaultdict(int)
    
    try:
        # Read lines from standard input
        for line in sys.stdin:
            # Strip whitespace and newlines
            line = line.strip()
            if line:  # Skip empty lines
                if line.startswith('timeit'):
                    _, b, c = line.split(' ')
                    line_counts[b] += float(c)+0.0000001
                
        return line_counts
    except KeyboardInterrupt:
        print("\nInput terminated by user.")
        return line_counts

def print_results(counts):
    """
    Prints the line counts in ascending order by count.
    If counts are equal, sorts by the line content.
    """
    # Convert to list of tuples and sort
    sorted_counts = sorted(counts.items(), key=lambda x: ( x[0], x[1]))
    
    # Print sorted results
    for line, count in sorted_counts:
        print(f"{line}: {count}")

def main():
    # Get the counts
    counts = count_lines()
    
    # Print results if we have any
    if counts:
        print_results(counts)
    else:
        print("No input provided.")

if __name__ == "__main__":
    main()