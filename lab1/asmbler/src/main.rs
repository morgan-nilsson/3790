use clap::Parser;


mod asmbler;
use asmbler::Asmbler;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    #[arg(short, long)]
    input_file: String,
    #[arg(short, long, default_value = "output.sml")]
    output_file: String,
}

fn main() {
    let args = Args::parse();

    let input = std::fs::read_to_string(&args.input_file)
        .expect("Failed to read input file");

    let result = Asmbler::assemble(input, vec![]);

    if result.has_errors() {
        result.print_errors();
    }

    std::fs::write(&args.output_file, result.contents)
        .expect("Failed to write output file");

    println!("Assembly successful! Output written to {}", args.output_file);

}