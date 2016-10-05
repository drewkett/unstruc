extern crate cgmath;

#[macro_use]
extern crate clap;

#[macro_use]
extern crate nom;

use cgmath::Point3;
use clap::{Arg, App, SubCommand};
use nom::{multispace,space,alphanumeric,IResult};

use std::str;
use std::io;
use std::path::Path;
use std::io::Read;
use std::fs::File;

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

// struct Grid {
//     points : Vec<Point3<f64>>,
//     elements : Vec<Element>,
//     names : Vec<Name>,
//     dim : Dimension,
// }

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

named!(read_solid< &[u8],Option<String> >,
       chain!(
           tag!("solid")~
               space~
               name:map!(map_res!(alphanumeric,str::from_utf8),str::to_string)?,
           ||{name}
       )
);

named!(parse_float<f64>,map_res!(map_res!(is_a!("0123456789eE-+."),str::from_utf8),str::parse::<f64>));

named!(read_vector< &[u8],Point3<f64> >,
       chain!(
           x:parse_float~
               space~
               y:parse_float~
               space~
               z:parse_float,
           ||Point3{x:x,y:y,z:z}));

named!(read_facet< &[u8],STLFacet >,
       chain!(
           multispace~
               tag!("facet")~
               space~
               tag!("normal")~
              space~ normal:read_vector~
               multispace~
               tag!("outer loop")~
               multispace~
               tag!("vertex")~
               space~
               p1:read_vector~
               multispace~
               tag!("vertex")~
               space~
               p2:read_vector~
               multispace~
               tag!("vertex")~
               space~
               p3:read_vector~
               multispace~
               tag!("endloop")~
               multispace~
               tag!("endfacet"),
           ||STLFacet{normal:normal,p1:p1,p2:p2,p3:p3}));

#[derive(Debug)]
struct STLFacet {
    normal : Point3<f64>,
    p1 : Point3<f64>,
    p2 : Point3<f64>,
    p3 : Point3<f64>
}

#[derive(Debug)]
struct STL {
    name : Option<String>,
    facets : Vec<STLFacet>
}

// Temporary until i set up functioning Grid functions
#[derive(Debug)]
struct Grid {
    stl : STL
}

named!(parse_stl< &[u8],STL >,
       chain!(
           name:read_solid~
               facets:many0!(read_facet)~
               multispace~ tag!("endsolid"),
           ||{STL{name:name,facets:facets}}
       ));

fn read_su2(filename : &str) -> io::Result<Grid> {
    return Err(io::Error::new(io::ErrorKind::NotFound,"SU2 Reader Not Implemented"));
}

fn read_stl(filename : &str) -> io::Result<Grid> {
    let mut f = try!(File::open(filename));
    let mut buffer = Vec::new();
    // Ideally this wouldn't need to read in whole file
    let res = f.read_to_end(&mut buffer);
    match res {
        Ok(_) => (),
        Err(e) => return Err(e)
    }
    let s = buffer.as_slice();
    println!("Reading STL");
    let res = parse_stl(s);
    println!("Finished Reading");
    match res {
        IResult::Done(_,stl) => return Ok(Grid{stl:stl}),
        IResult::Error(_) => return Err(io::Error::new(io::ErrorKind::NotFound,format!("Error Parse STL file {:?}",filename))),
        IResult::Incomplete(_) => return Err(io::Error::new(io::ErrorKind::NotFound,format!("STL file truncated {:?}",filename)))
    }
}

fn read_stl_binary(filename : &str) -> io::Result<Grid> {
    return Err(io::Error::new(io::ErrorKind::NotFound,"STL Binary Reader Not Implemented"));
}

fn read_grid(surface_file : &str) -> io::Result<Grid> {
    let filetype = filetype_from_filename(surface_file);
    match filetype {
        Some(FileType::SU2) => read_su2(surface_file),
        Some(FileType::STL) => read_stl(surface_file),
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
            Ok(g) => println!("{:?}",surface_file),
            Err(e) => println!("{}",e),
        }
    }
}
