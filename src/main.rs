extern crate cgmath;
#[macro_use]
extern crate clap;

use cgmath::Point3;
use clap::{Arg, App, SubCommand};
use std::path::Path;

enum ShapeType {
    Undefined,
    Line,
    Triangle,
    Quad,
    Polygon,
    Tetra,
    Hexa,
    Wedge,
    Pyramid,
    NShapes,
}

enum Dimension {
    Two,
    Three,
}

struct Element {
    shape_type : ShapeType,
    name_i : i16,
    points : Vec<u32>,
}

struct Name {
    dim : Dimension,
    name : String,
}

struct Grid {
    points : Vec<Point3<f64>>,
    elements : Vec<Element>,
    names : Vec<Name>,
    dim : Dimension,
}

enum FileType {
    SU2,
    STL,
    STLBinary,
    VTK,
    Plot3D
}

fn filetype_from_filename(filename : &str) -> Option<FileType> {
    if let Some(ext) = Path::new(filename).extension() {
        match ext.to_str() {
            Some("su2") => Some(FileType::SU2),
            Some("stl") => Some(FileType::STL),
            Some("stlb") => Some(FileType::STLBinary),
            Some("vtk") => Some(FileType::VTK),
            Some("xyz") => Some(FileType::Plot3D),
            Some("p3d") => Some(FileType::Plot3D),
            _ => None
        }
    } else {
        None
    }
}

fn read_su2(filename : &str) -> Option<Grid> {
    None
}

fn read_stl(filename : &str) -> Option<Grid> {
    None
}

fn read_stl_binary(filename : &str) -> Option<Grid> {
    None
}

fn read_grid(surface_file : &str) -> Option<Grid> {
    let filetype = filetype_from_filename(surface_file);
    match filetype {
        Some(FileType::SU2) => read_su2(surface_file),
        Some(FileType::STL) => read_stl(surface_file),
        Some(FileType::STLBinary) => read_stl_binary(surface_file),
        _ => None
    }
}

fn main() {
    let matches = App::new("unstruc")
        .version("2.0 Alpha")
        .about("Unstructured grid tool")
        .subcommand(SubCommand::with_name("offset")
                    .about("Creates hybrid grid using stl file")
                    .version("2.0 Alpha")
                    .arg(Arg::with_name("growth_rate")
                         .short("-g")
                         .long("--growth-rate")
                         .takes_value(true)
                         .default_value("1.5"))
                    .arg(Arg::with_name("number_of_layers")
                         .short("-n")
                         .long("--number-of-layers")
                         .takes_value(true)
                         .default_value("1"))
                    .arg(Arg::with_name("offset_size")
                         .short("-s")
                         .long("--offset-size")
                         .takes_value(true)
                         .required(true))
                    .arg(Arg::with_name("surface_file")
                         .required(true)
                         .help("Surface File (.stl)"))
                    .arg(Arg::with_name("output_file")
                         .required(true)
                         .help("Output File"))
        )
        .get_matches();

    if let Some(sub_m) = matches.subcommand_matches("offset") {
        let surface_file = sub_m.value_of("surface_file").unwrap();
        let growth_rate = value_t!(sub_m.value_of("growth_rate"),f64).unwrap_or_else(|e| e.exit());
        let number_of_layers = value_t!(sub_m.value_of("number_of_layers"),u32).unwrap_or_else(|e| e.exit());
        let offset_size = value_t!(sub_m.value_of("offset_size"),f64).unwrap_or_else(|e| e.exit());
        println!("{}",surface_file)
    }
}
