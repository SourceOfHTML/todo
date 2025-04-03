use std::{
    fs::{File, OpenOptions},
    io::{Read, Write},
    path::Path,
};

use clap::{Parser, Subcommand};
use colored::Colorize;

/// Sigma todo app
///
/// if no subcommand is provided it will default to listing all added todos
#[derive(Parser)]
struct Cli {
    #[command(subcommand)]
    command: Option<Commands>,
}

#[derive(Subcommand)]
enum Commands {
    /// List all todos
    List,
    /// Add new todo
    Add { name: String, description: String },
    /// Remove todo using id
    Remove { id: u32 },
}

struct Todo {
    id: u32,
    name: String,
    desc: String,
    // is_finished: bool // the unsigma c writer didn't actually use this lmaoooo
}
impl Todo {
    pub fn from_str_line(line: &str) -> Option<Self> {
        let mut split_line = line.split(":");

        let id = split_line.next()?.parse::<u32>().ok()?;

        let line = split_line.collect::<Vec<&str>>().join(":");
        let title_desc_split: Vec<&str> = line.split('\u{1F}').collect();
        let name = title_desc_split.get(0)?.trim().to_string();
        let desc = title_desc_split.get(1)?.replace("\\n", "\n");

        Some(Self { id, name, desc })
    }

    pub fn to_str_line(&self) -> String {
        format!(
            "{}:{}\u{1F}{}",
            self.id,
            self.name,
            self.desc.replace("\n", "\\n")
        )
    }
}

fn main() {
    let cli = Cli::parse();
    match cli.command {
        None | Some(Commands::List) => list(),
        Some(Commands::Add { description, name }) => add(description, name),
        Some(Commands::Remove { id }) => remove(id),
    }
}

fn list() {
    let mut file = File::open(Path::new("./todo_list")).expect("File doesn't exists!!!");
    let mut buf = String::new();
    file.read_to_string(&mut buf).unwrap();
    let todos: Vec<Todo> = buf
        .split("\n")
        .filter_map(|x| Todo::from_str_line(x))
        .collect();
    todos
        .iter()
        .for_each(|x| println!("{}: {}\n{}", x.id, x.name.bold(), x.desc));
}

fn add(description: String, name: String) {
    let path = Path::new("./todo_list");
    let mut id = 0;
    if path.exists() {
        let mut file = File::open(path).expect("File doesn't exists!!!");
        let mut buf = String::new();
        file.read_to_string(&mut buf).unwrap();
        let todos: Vec<Todo> = buf
            .split("\n")
            .filter_map(|x| Todo::from_str_line(x))
            .collect();
        id = todos.iter().last().map_or(0, |x| x.id + 1); // map_or because file could exist but be empty
    }
    let todo = Todo {
        id,
        name,
        desc: description,
    };
    let todo_line = todo.to_str_line();
    let mut file = OpenOptions::new()
        .create(true)
        .write(true)
        .append(true)
        .open(path)
        .unwrap();
    writeln!(file, "{todo_line}").unwrap();
}

fn remove(id: u32) {
    let path = Path::new("./todo_list");
    if !path.exists() {
        println!("no todos made yet!!!");
        return;
    }
    let mut file = File::open(path).unwrap();
    let mut buf = String::new();
    file.read_to_string(&mut buf).unwrap();
    let todos: Vec<Todo> = buf
        .split("\n")
        .filter_map(|x| Todo::from_str_line(x))
        .collect();
    let mut exists = false;
    let new_lines: Vec<String> = todos
        .iter()
        .filter_map(|x| {
            if x.id == id {
                exists = true;
                None
            } else {
                Some(x.to_str_line())
            }
        })
        .collect();
    if exists {
        let mut file = File::create(path).unwrap();
        writeln!(file, "{}", new_lines.join("\n")).unwrap();
    } else {
        println!("todo doesn't exist!!!")
    }
}
