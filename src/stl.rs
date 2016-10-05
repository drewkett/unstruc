use cgmath::Point3;
use nom::{multispace,space,alphanumeric,IResult};

use std::str;
use std::io;
use std::path::Path;
use std::io::Read;
use std::fs::File;
use std::fmt;

struct Facet {
    normal : Point3<f64>,
    p1 : Point3<f64>,
    p2 : Point3<f64>,
    p3 : Point3<f64>
}

pub struct STL {
    name : Option<String>,
    facets : Vec<Facet>
}
impl fmt::Display for STL {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        if let Some(ref name) = self.name {
            write!(f,"STL(name={},facets={})",name,self.facets.len())
        } else {
            write!(f,"STL(facets={})",self.facets.len())
        }
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

named!(read_vertex< &[u8], Point3<f64> >,
       chain!(
           tag!("vertex")~
               space~
               p:read_vector,
           ||{p}
       )
);

named!(read_facet< &[u8],Facet >,
       chain!(
           multispace~
               tag!("facet")~ space~
               tag!("normal")~ space~
               normal:read_vector~ multispace~
               tag!("outer loop")~ multispace~
               p1:read_vertex~ multispace~
               p2:read_vertex~ multispace~
               p3:read_vertex~ multispace~
               tag!("endloop")~ multispace~
               tag!("endfacet"),
           ||Facet{normal:normal,p1:p1,p2:p2,p3:p3}));

named!(parse_stl< &[u8],STL >,
       chain!(
           name:read_solid~
               facets:many0!(read_facet)~
               multispace~ tag!("endsolid"),
           ||{STL{name:name,facets:facets}}
       ));

pub fn read_file(filename : &str) -> io::Result<STL> {
    let mut f = try!(File::open(filename));
    let mut buffer = Vec::new();
    // Ideally this wouldn't need to read in whole file
    let res = f.read_to_end(&mut buffer);
    match res {
        Ok(_) => (),
        Err(e) => return Err(e)
    }
    let s = buffer.as_slice();
    let res = parse_stl(s);
    match res {
        IResult::Done(_,stl) => return Ok(stl),
        IResult::Error(_) => return Err(io::Error::new(io::ErrorKind::NotFound,format!("Error Parse STL file {:?}",filename))),
        IResult::Incomplete(_) => return Err(io::Error::new(io::ErrorKind::NotFound,format!("STL file truncated {:?}",filename)))
    }
}
