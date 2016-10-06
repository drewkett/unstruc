extern crate cgmath;

#[macro_use]
extern crate clap;

#[macro_use]
extern crate nom;

use cgmath::Point3;
use clap::{Arg, App, SubCommand};

use std::fmt;
use std::str;
use std::io;
use std::path::Path;

mod stl;

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

impl fmt::Display for Dimension {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match *self {
            Dimension::Two => write!(f,"2"),
            Dimension::Three => write!(f,"3")
        }
    }
}

struct Element {
    shape_type : ShapeType,
    name_i : i16,
    points : Vec<usize>,
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

impl fmt::Display for Grid {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f,
               "Grid(points={},elements={},dim={})",
               self.points.len(),
               self.elements.len(),
               self.dim)
    }
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

fn read_su2(filename : &str) -> io::Result<Grid> {
    return Err(io::Error::new(io::ErrorKind::NotFound,"SU2 Reader Not Implemented"));
}

fn read_stl_binary(filename : &str) -> io::Result<Grid> {
    return Err(io::Error::new(io::ErrorKind::NotFound,"STL Binary Reader Not Implemented"));
}

fn stl_to_grid(s : stl::STL) -> Grid {
    let mut points = vec![];
    let mut elements = vec![];
    let mut names = vec![];
    if let Some(name) = s.name {
        names.push(Name{dim:Dimension::Two,name:name});
    }
    for f in s.facets {
        let i = points.len();
        elements.push(
            Element{
                shape_type:ShapeType::Triangle,
                name_i:0,
                points:vec![i,i+1,i+2]
            }
        );
        points.push(f.p1);
        points.push(f.p2);
        points.push(f.p3);
    }
    return Grid{points:points,elements:elements,names:names,dim:Dimension::Three}
}

fn read_grid(surface_file : &str) -> io::Result<Grid> {
    let filetype = filetype_from_filename(surface_file);
    match filetype {
        Some(FileType::SU2) => read_su2(surface_file),
        Some(FileType::STL) => stl::read_file(surface_file).map(stl_to_grid),
        Some(FileType::STLBinary) => read_stl_binary(surface_file),
        Some(_) => Err(io::Error::new(io::ErrorKind::NotFound,"Unknown File Type")),
        None => Err(io::Error::new(io::ErrorKind::NotFound,"Unknown File Type"))
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
        match read_grid(surface_file) {
            Ok(g) => println!("{}",g),
            Err(e) => println!("{}",e),
        }
    }
}
