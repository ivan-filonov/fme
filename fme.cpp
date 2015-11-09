#include <algorithm>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <stdio.h>

using Path = std::vector<std::string>;
using PathIterator = Path::iterator;

static Path splitPath(const std::string &path) {
  Path result;
  if (path.empty() || '/' != path[0]) {
    return result;
  }

  size_t start = 1;
  for (;;) {
    auto stop = path.find('/', start);
    if (std::string::npos != stop) {
      result.emplace_back(path.substr(start, stop - start));
      if (result.back().empty()) {
        result.clear();
        break;
      }
      start = stop + 1;
    } else {
      result.emplace_back(path.substr(start));
      break;
    }
  }

  return result;
}

class Node;
using Nodes = std::vector<Node>;

class Node {
private:
  std::string name_;
  bool isDirectory_;
  Nodes nodes_;

public:
  bool isDirectory() const { return isDirectory_; }
  Nodes &nodes() { return nodes_; }

  bool operator<(const Node &other) const { return name_ < other.name_; }
  bool operator<(const std::string &otherName) const {
    return name_ < otherName;
  }

  Node(const std::string &name, bool isDirectory)
      : name_(name), isDirectory_(isDirectory) {}

  Node(const std::string &name, bool isDirectory, Nodes &&nodes)
      : name_(name), isDirectory_(isDirectory), nodes_(std::move(nodes)) {}

  Node *find(const std::string &name_);
  Node *find(PathIterator begin, PathIterator end);
  Node *add(std::string name_, bool isDirectory_, Nodes &&nodes_);
  void remove(const std::string &name_);
  void printTree(const std::string &prefix) const;
};

void Node::printTree(const std::string &prefix) const {
  std::string newPrefix = prefix;
  if (!prefix.empty()) {
    printf("%s_%s%s\n", &prefix[0], &name_[0], isDirectory_ ? "/" : "");
    // printf("%s_%s\n", &prefix[0], &name[0]);
    newPrefix.append(" |");
  } else {
    printf("/\n");
    newPrefix = "|";
  }
  for (const auto &subNode : nodes_) {
    subNode.printTree(newPrefix);
  }
}

void Node::remove(const std::string &name) {
  auto pos = std::lower_bound(nodes_.begin(), nodes_.end(), name);
  if (nodes_.end() != pos && pos->name_ == name) {
    nodes_.erase(pos);
  }
}

Node *Node::add(std::string name, bool isDirectory, Nodes &&nodes) {
  auto pos = std::lower_bound(nodes_.begin(), nodes_.end(), name);
  if (nodes_.end() != pos && name == pos->name_) {
    return nullptr;
  }
  auto inserted = nodes_.insert(pos, Node{name, isDirectory, std::move(nodes)});
  return &inserted[0];
}

Node *Node::find(PathIterator begin, PathIterator end) {
  auto result = this;
  while (begin != end) {
    auto next = result->find(*begin);
    if (nullptr == next || !next->isDirectory_) {
      return nullptr;
    }
    result = next;
    ++begin;
  }
  return result;
}

Node *Node::find(const std::string &name) {
  auto pos = std::lower_bound(nodes_.begin(), nodes_.end(), name);
  Node *result = nullptr;
  if (nodes_.end() != pos && pos->name_ == name) {
    result = &pos[0];
  }
  return result;
}

class Main {
public:
  using CommandFunction = std::function<bool(Main *, std::vector<Path> &)>;

  std::unordered_map<std::string, CommandFunction> commands{
      {"md", &Main::makeDirectoryNode}, {"mf", &Main::makeFileNode},
      {"rm", &Main::removeNode},        {"cp", &Main::copyNode},
      {"mv", &Main::moveNode},
  };

  Node root{"", true};

  bool checkArgCount(const std::vector<Path> &args, int expected,
                     const char *command);
  /*
   *
   *     md – creates a directory.
   *
   *     Command format: md <path>
   *
   *     Notes: md should not create any intermediate directories in the path.
   *
   *     Examples:
   *
   *     a)      md /Test – creates a directory called Test in the root
   * directory.
   *
   *     b)      md /Dir1/Dir2/NewDir – creates a subdirectory “NewDir” if
   * directory “/Dir1/Dir2” exists.
   */
  bool makeDirectoryNode(std::vector<Path> &args);
  /*
   *
   *     mf – creates a file.
   *
   *     Command format: mf <path>
   *
   *     Notes: if such file already exists with the given path then FME should
   * continue to the next command in the batch file without any error rising.
   *
   *     Examples:
   *
   *     mf /Dir2/Dir3/file.txt – creates a file named file.txt in “/Dir2/Dir3”
   * subdirectory.
   * */
  bool makeFileNode(std::vector<Path> &args);
  /*
   *
   *     rm – removes a file or a directory with all its contents.
   *
   *     Command format: rm <path>
   *
   *     Examples:
   *
   *     a)      rm /Dir2/Dir3 – removes the directory “/Dir2/Dir3”.
   *
   *     b)      rm /Dir2/Dir3/file.txt – removes the file “file.txt” from the
   * directory “/Dir2/Dir3”.
   * */
  bool removeNode(std::vector<Path> &args);
  /*
   *
   *     cp – copy an existed directory/file to another location.
   *
   *     Command format: cp <source> <destination>
   *
   *     Notes: Program should copy directory with all its content. Destination
   * path should not contain any file name except base-name otherwise FME should
   * raise error (Base-name of “/dir/file.txt” is “file.txt”).
   *
   *     Examples:
   *
   *     a)      cp /Dir2/Dir3 /Dir1 – copies directory Dir3 in /Dir2 to /Dir1.
   *
   *     b)      cp /Dir2/Dir3/file.txt /Dir1 – copies file “file.txt” from
   * /Dir2/Dir3 to /Dir1.
   *
   *     c)      cp /Dir2/Dir3/file.txt /Dir1/newfile.txt – copies file
   * “file.txt” from /Dir2/Dir3 to /Dir1 and renames it to “newfile.txt”.
   */
  bool copyNode(std::vector<Path> &args);
  /*
   *
   *     mv – moves an existing directory/file to another location
   *
   *     Command format: mv <source> <destination>
   *
   *     Notes: Program should move directory with all its content.
   */
  bool moveNode(std::vector<Path> &args);

  bool processCmdLine(std::string &cmdLine);
  int run(int argc, char **argv);
};

bool Main::checkArgCount(const std::vector<Path> &args, int expected,
                         const char *command) {
  if ((int)args.size() != expected) {
    printf("ERROR: %s - invalid number of arguments, expected %d, got %lu",
           command, expected, args.size());
    return false;
  }
  return true;
}

bool Main::makeDirectoryNode(std::vector<Path> &args) {
  if (!checkArgCount(args, 1, "md")) {
    return false;
  }
  auto &path = args[0];
  auto end = path.end() - 1;
  auto dstDir = root.find(path.begin(), end);
  if (nullptr == dstDir) {
    printf("ERROR: md should not create any intermediate directories in "
           "the path.");
    return false;
  }

  auto dstNode = dstDir->find(*end);
  if (nullptr == dstNode) {
    dstDir->add(*end, true, {});
  } else if (!dstNode->isDirectory()) {
    printf("ERROR: cannot create directory, because file with the same name "
           "already exist");
    return false;
  }
  return true;
}

bool Main::makeFileNode(std::vector<Path> &args) {
  if (!checkArgCount(args, 1, "mf")) {
    return false;
  }
  auto &path = args[0];
  auto end = path.end() - 1;
  auto dstDir = root.find(path.begin(), end);
  if (nullptr == dstDir) {
    printf("ERROR: mf should not create any intermediate directories in "
           "the path.");
    return false;
  }

  auto dstNode = dstDir->find(*end);
  if (nullptr == dstNode) {
    dstDir->add(*end, false, {});
  } else if (!dstNode->isDirectory()) {
    printf("ERROR: cannot create file, because directory with the same name "
           "already exist");
    return false;
  }
  return true;
}

bool Main::removeNode(std::vector<Path> &args) {
  if (!checkArgCount(args, 1, "rm")) {
    return false;
  }
  auto &path = args[0];
  if (path.size() == 1 && path.back().empty()) {
    printf("ERROR: removal of root is not allowed");
    return false;
  }
  auto end = path.end() - 1;
  auto dstDir = root.find(path.begin(), end);
  if (nullptr == dstDir || nullptr == dstDir->find(*end)) {
    printf("ERROR: file or directory doesn't exist");
    return false;
  }
  dstDir->remove(*end);
  return true;
}

bool Main::copyNode(std::vector<Path> &args) {
  if (!checkArgCount(args, 2, "cp")) {
    return false;
  }
  // 1. find source node and its container
  auto &srcPath = args[0];
  auto srcEnd = srcPath.end() - 1;
  auto srcDir = root.find(srcPath.begin(), srcEnd);
  if (nullptr == srcDir) {
    printf("ERROR: file or directory doesn't exist");
    return false;
  }

  Node *srcNode = srcDir->find(*srcEnd);
  if (nullptr == srcNode) {
    printf("ERROR: file or directory doesn't exist");
    return false;
  }

  // 2. find destination node
  auto &dstPath = args[1];
  auto dstEnd = dstPath.end() - 1;
  auto dstDir = root.find(dstPath.begin(), dstEnd);
  if (nullptr == dstDir) {
    printf("ERROR: destination directory doesn't exist");
    return false;
  }

  Node *dstNode = dstDir->find(*dstEnd);
  if (nullptr != dstNode) {
    if (dstNode->isDirectory()) {
      dstDir = dstNode;
    } else {
      printf("ERROR: file already exist at copy destination");
      return false;
    }
  }

  // 3. copy
  auto newName = *srcEnd;
  if (nullptr == dstNode) {
    // have to rename and copy
    newName = *dstEnd;
  }
  dstDir->add(newName, srcNode->isDirectory(), Nodes(srcNode->nodes()));

  return true;
}

bool Main::moveNode(std::vector<Path> &args) {
  if (!checkArgCount(args, 2, "mv")) {
    return false;
  }
  // 1. find source node and its container
  auto &srcPath = args[0];
  auto srcEnd = srcPath.end() - 1;
  auto srcDir = root.find(srcPath.begin(), srcEnd);
  if (nullptr == srcDir) {
    printf("ERROR: file or directory doesn't exist");
    return false;
  }

  Node *srcNode = srcDir->find(*srcEnd);
  if (nullptr == srcNode) {
    printf("ERROR: file or directory doesn't exist");
    return false;
  }

  // 2. find destination node
  auto &dstPath = args[1];
  auto dstEnd = dstPath.end() - 1;
  auto dstDir = root.find(dstPath.begin(), dstEnd);
  if (nullptr == dstDir) {
    printf("ERROR: destination directory doesn't exist");
    return false;
  }

  Node *dstNode = dstDir->find(*dstEnd);
  if (nullptr != dstNode) {
    if (dstNode->isDirectory()) {
      dstDir = dstNode;
    } else {
      printf("ERROR: file already exist at move destination");
      return false;
    }
  }

  // 3. move
  auto newName = *srcEnd;
  if (nullptr == dstNode) {
    newName = *dstEnd;
  }
  bool isDirectory = srcNode->isDirectory();
  Nodes nodes = std::move(srcNode->nodes());

  srcDir->remove(*srcEnd);
  dstDir->add(newName, isDirectory, std::move(nodes));

  return true;
}

bool Main::processCmdLine(std::string &cmdLine) {
  std::istringstream ss{cmdLine};
  std::string cmd;
  if (!(ss >> cmd)) {
    printf("ERROR: empty command");
    return false;
  }

  auto iter = commands.find(cmd);
  if (commands.end() == iter) {
    printf("ERROR: unknown command - %s", &cmd[0]);
    return false;
  }

  std::vector<Path> args;
  for (std::string arg; ss >> arg;) {
    args.push_back(splitPath(arg));
    if (args.back().empty()) {
      printf("ERROR: invalid path - '%s'", &arg[0]);
      return false;
    }
  }

  return iter->second(this, args);
}

int Main::run(int argc, char **argv) {
  std::ifstream src(argv[1]);
  // std::ifstream src(2 == argc ? argv[1] : "test1.txt");
  bool success = true;
  for (std::string cmdLine; std::getline(src, cmdLine);) {
    success = processCmdLine(cmdLine);
    if (!success) {
      break;
    }
  }

  if (success) {
    root.printTree("");
  }

  return 0;
}

int main(int argc, char **argv) {
  if (2 != argc) {
    printf("Usage: %s <batch-file>", argv[0]);
    return 1;
  }

  Main app;
  return app.run(argc, argv);
}
