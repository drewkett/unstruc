extern crate gcc;

fn main() {
    gcc::Config::new()
        .cpp(true)
        .file("src/tetgen1.5.0/tetgen.cxx")
        .file("src/tetgen1.5.0/predicates.cxx")
        .file("src/tetgen1.5.0/cinterface.cxx")
        .define("TETLIBRARY",None)
        .compile("libtetgen.a");
}
