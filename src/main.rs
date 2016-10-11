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
use std::cmp::Ordering;

mod stl;
mod tetgen;

pub enum Dimension {
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

pub enum Element {
    Triangle(usize,[usize;3]),
    Quad(usize,[usize;4]),
}

impl <'a> Element {
    fn iter_points(&'a self) -> std::slice::Iter<'a,usize> {
        return match self {
            &Element::Triangle(_,ref points) => points.iter(),
            &Element::Quad(_,ref points) => points.iter()
        }
    }

    fn iter_points_mut(&'a mut self) -> std::slice::IterMut<'a,usize> {
        return match self {
            &mut Element::Triangle(_,ref mut points) => points.iter_mut(),
            &mut Element::Quad(_,ref mut points) => points.iter_mut()
        }
    }
}

pub struct Name {
    dim : Dimension,
    name : String,
}

pub struct Grid {
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

impl Grid {
    fn merge_duplicates(&mut self, tol : f64) -> &mut Grid {
        let mut v = Vec::<PointSorter>::with_capacity(self.points.len());
        for (i,p) in self.points.iter().enumerate() {
            let l = p.x + p.y + p.z;
            v.push(PointSorter{l:l,x:p.x,y:p.y,z:p.z,i:i});
        }
        v.sort_by(|a,b| a.partial_cmp(b).unwrap_or(Ordering::Less));

        let mut n_merged = 0;
        for i in 0..v.len() {
            for j in (i+1)..v.len() {
                if v[j].i == v[i].i {
                    continue;
                }
                if v[j].l - v[i].l > 3.*tol {
                    break;
                }
                if (v[i].x - v[j].x).abs() < tol &&
                    (v[i].y - v[j].y).abs() < tol &&
                    (v[i].z - v[j].z).abs() < tol {
                        v[j].i = v[i].i;
                        n_merged += 1;
                    }
            }
        }
        for e in self.elements.iter_mut() {
            for p in e.iter_points_mut() {
                *p = v[*p].i
            }
        }
        return self
    }

    fn delete_dead_points(&mut self) -> &mut Grid {
        let n = self.points.len();
        let mut seen = vec![false; n];
        for e in self.elements.iter() {
            for &p in e.iter_points() {
                seen[p] = true;
            }
        }
        let n_seen_points = seen.iter().fold(0, |acc, &b| acc + b as usize);

        let mut new_index : Vec<usize> = (0..n).collect();
        let mut new_points = Vec::<Point3<f64>>::with_capacity(n_seen_points);
        let mut j = 0;
        let mut dead_points = 0;
        for (i,&b) in seen.iter().enumerate() {
            if b {
                new_index[i] = j;
                new_points.push(self.points[i]);
                j += 1;
            } else {
                dead_points += 1;
            }
        }
        for mut e in self.elements.iter_mut() {
            for p in e.iter_points_mut() {
                *p = new_index[*p];
            }
        }
        self.points = new_points;
        return self;
    }

    fn check(&self) -> bool {
        let n = self.points.len();
        for e in self.elements.iter() {
            for &p in e.iter_points() {
                if p >= n {
                    return false;
                }
            }
        }
        return true;
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
            Element::Triangle(0,[i,i+1,i+2])
        );
        points.push(f.p1);
        points.push(f.p2);
        points.push(f.p3);
    }
    return Grid{points:points,elements:elements,names:names,dim:Dimension::Three}
}

#[derive(PartialEq,PartialOrd,Debug)]
struct PointSorter {
    l : f64,
    x : f64,
    y : f64,
    z : f64,
    i : usize
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
        let mut g = read_grid(surface_file).unwrap();
        println!("{}",g);
        let valid = g.merge_duplicates(1e-5).delete_dead_points().check();
        println!("{}",g);
        if valid {
            println!("Valid");
        }
        let tg = tetgen::TetGenIO::new(g);
    }
}
