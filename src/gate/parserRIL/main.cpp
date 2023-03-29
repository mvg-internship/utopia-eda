#include <iostream>
#include <fstream>
#include "passes/techmap/libparse.h"
#include <kernel/yosys.h>
#include "gate/model/gate.h"
#include "gate/model/gnet.h"
#include "base/model/signal.h"
#include "gate/simulator/simulator.h"
#include <map>
#include <string>
#include <algorithm>

using namespace eda::gate::model;
std::map<int, std::string> INPUTS;
std::map<int, std::string> OUTPUTS;
std::map<int, std::string > typpfunc;
std::vector<int> WIRES;
std::vector<int> seq;
std::vector<int> clk;

void print_modules(const int ind, std::ostream &out, std::ofstream &fout){
  for (const auto &it1 : Yosys::RTLIL::IdString::global_id_index_){
    if (it1.second == ind){
      out << it1.first << " - module of index: " << it1.second << "\n";
    }
    INPUTS.clear();
    OUTPUTS.clear();
    typpfunc.clear();
    WIRES.clear();
    seq.clear();
    clk.clear();

  }
}


std::map<int,std::string> wires1;
void print_wires(const Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Wire*> &wires, std::ostream &out, std::ofstream &fout, eda::gate::model::GNet &net,std::map<unsigned, Gate::Id> &inputs,std::map<unsigned, Gate::Id> &outputs){
  out << "Wires:" << "\n";
  for (auto it1=wires.begin(); it1 != wires.end(); ++it1){
    std::string temp;
    std::string width = "u:";
    unsigned index;
    for(const auto &it2 : Yosys::RTLIL::IdString::global_id_index_){
      if (it2.second == it1->first.index_){
        out << "  " << it2.first << " - wire of index: " << it2.second << "\n";
        index=it2.second;
        temp=it2.first;
      }
    }
    width.append(std::to_string(it1->second->width));
    if (it1->second->width>1){
      out<< "    type: bus with width " << it1->second->width << "\n";
      fout << "output u:" << it1->second->width<< " " << temp << ";\n";
      OUTPUTS.emplace(index, temp);
    }
    else{
      out << "    type: wire" << "\n";
    }
    out << "    start_offset: " << it1->second->start_offset << "\n";
    out << "    port_id: " << it1->second->port_id << "\n";
    out << "    port_input: " << it1->second->port_input << "\n";
    if (it1->second->port_input==1){
      Gate::Id inputId = net.addIn();
      inputs.emplace(index, inputId);
      temp.erase(0, 1);
      fout << "input " << width << " " << temp << ";\n";
      INPUTS.emplace(index, temp);
    }
    out << "    port_output: " << it1->second->port_output << "\n";
    if (it1->second->port_output==1){
      Gate::Id outputId;
      outputs.emplace(index, outputId);
      temp.erase(0, 1);
      fout << "output " << width << " " << temp << ";\n";
      OUTPUTS.emplace(index, temp);
    }
    if (it1->second->port_output==0 && it1->second->port_input==0){
      WIRES.push_back(index);
      std::string z=temp;
      wires1.emplace(index, z);
      fout <<"wire "<<width<<" "<<temp <<";\n";
    }
    out << "    upto: " << it1->second->upto << "\n";
    out << "\n";
  }
}
std::string function(size_t type){
  std::string func;

  if (type==ID($and).index_){
    func = "&";
  }
  if (type==ID($not).index_){
    func = "~";
  }
  if (type==ID($or).index_){
    func = "|";
  }
  if (type==ID($xor).index_){
    func="^";
  }
  return func;
}

std::map<int, std::pair<int, int>> CELL;
std::string buildRIL(int root){
  bool flag1=0,flag2=0;
  if(INPUTS.find(CELL.find(root)->second.first)!=INPUTS.end()){
    flag1=1;
  }
  if(INPUTS.find(CELL.find(root)->second.second)!=INPUTS.end()){
    flag2=1;
  }
  if (flag1==1&&flag2==1){
    if (CELL.find(root)->second.first!=CELL.find(root)->second.second){
      return INPUTS.find(CELL.find(root)->second.first)->second + typpfunc.find(CELL.find(root)->first)->second + INPUTS.find(CELL.find(root)->second.second)->second;
    }
    else{
      return typpfunc.find(CELL.find(root)->first)->second + INPUTS.find(CELL.find(root)->second.second)->second;
    }
  }
  if (flag1==0&&flag2==0){
    if (CELL.find(root)->second.first!=CELL.find(root)->second.second){
      return "(" + buildRIL(CELL.find(root)->second.first) + typpfunc.find(CELL.find(root)->first)->second + buildRIL(CELL.find(root)->second.second) + ")";
    }
    else{
      return typpfunc.find(CELL.find(root)->first)->second+"(" + buildRIL(CELL.find(root)->second.first) + ")";
    }

  }
  if (flag1==0&&flag2==1){
    return "(" + buildRIL(CELL.find(root)->second.first) + typpfunc.find(CELL.find(root)->first)->second + INPUTS.find(CELL.find(root)->second.second)->second + ")";
  }
  if (flag1==1&&flag2==0){
    return "(" + INPUTS.find(CELL.find(root)->second.first)->second + typpfunc.find(CELL.find(root)->first)->second + buildRIL(CELL.find(root)->second.second) + ")";
  }
}

void print_cells(const Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Cell*> &cells, std::ostream &out, std::ofstream &fout, eda::gate::model::GNet &net, std::map<int,GateSymbol> &typeFunc,std::map<int, std::pair<int, int>> &cell){
  out << "Cells:" << "\n";
  for (auto it1=cells.begin(); it1 != cells.end(); ++it1){
    for (const auto &it2 : Yosys::RTLIL::IdString::global_id_index_){
      if (it2.second == it1->first.index_){
        out << "  " << it2.first << " cell of index " << it2.second << " type " << it1->second->type.index_ << "\n";
      }
    }
    size_t i=0;
    bool flag=0;
    //GateSymbol f;
    std::string symbol;
    int a,b,c;
    for (auto it3 = it1->second->connections_.begin(); it3 != it1->second->connections_.end(); ++it3){
      i++;
      if (i==1){
        symbol = function(it1->second->type.index_);
        a=it3->second.as_wire()->name.index_;
        symbol=function(it1->second->type.index_);
        typpfunc.emplace(a,symbol);
        if (symbol=="~"){
          flag=1;
        }
      }
      if (i==2){
        if (!flag){
          b=it3->second.as_wire()->name.index_;
          flag=0;
        }
        else{
          b=it3->second.as_wire()->name.index_;
          c=b;
          CELL.emplace(a,std::make_pair(b,c));
        }
      }
      if (i==3){
        c=it3->second.as_wire()->name.index_;
        CELL.emplace(a,std::make_pair(b,c));
      }
      if (i==4){
        a=it3->second.as_wire()->name.index_;
      }
      if (i==5){
        b=it3->second.as_wire()->name.index_;
        CELL.emplace(c,std::make_pair(a,b));
        typpfunc.emplace(c,symbol);
      }
      out << "    Connections: " << it3->first.index_ << "\n";
      out << "      name: " << it3->second.as_wire()->name.index_ << "\n";
    }
  }
}
void print_connections(const std::vector<std::pair<Yosys::RTLIL::SigSpec, Yosys::RTLIL::SigSpec> > &connections, std::ostream &out, std::ofstream &fout, eda::gate::model::GNet &net,std::map<int,GateSymbol> typeFunc,std::map<int, std::pair<int, int>> &cell,std::map<unsigned, Gate::Id> &inputs,std::map<unsigned, Gate::Id> &outputs){
  out << "Connections count: " << connections.size() << "\n";
  int root;
  for (auto it1 = connections.begin(); it1 != connections.end(); ++it1){
    out << "    Wire " << it1->first.as_wire()->name.index_ << " connects with " << it1->second.as_wire()->name.index_ << "\n";
    if(inputs.find(it1->second.as_wire()->name.index_)==inputs.end()){
      root=it1->second.as_wire()->name.index_;
      fout << "@(*) {\n";
      fout << "   " << OUTPUTS.find(it1->first.as_wire()->name.index_)->second << " = " << buildRIL(root) << ";\n";
      fout << "}\n";
    }
    else{
//      fout << "@(*) {\n";
//      fout << "   " << OUTPUTS.find(it1->first.as_wire()->name.index_)->second << " = " << INPUTS.find(it1->second.as_wire()->name.index_)->second << ";\n";
//      fout << "}\n";
    }
  }
}

void print_memory(const Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Memory*> &memories, std::ostream &out, std::ofstream &fout){
  out << "Memory count: " << memories.size() << "\n";

  for (auto it1 = memories.begin(); it1 != memories.end(); ++it1){

    out << "  memory " << it1->second << "\n";
    out << "  name: " << it1->second->name.index_ << "\n";
    out << "  width: " << it1->second->width << "\n";
    out << "  start_offset: " << it1->second->start_offset << "\n";
    out << "  size: " << it1->second->size << "\n";
    out << "\n";
  }
}



void print_actions(const std::vector< Yosys::RTLIL::SigSig > &actions, std::ostream &out, std::ofstream &fout){
  int z=0;
  out << "actions:\n";
  for (auto &it : actions){
    z++;
    out << z <<"\n";
    out << "    ch" << (it.first.is_wire())<< " "<<(it.second.is_wire())<< "\n";
    if (it.first.is_wire() && it.second.is_wire()){
      out << "    nech" << (it.first.as_wire()->name.index_)<< " "<<(it.second.as_wire()->name.index_)<< "\n";
      if (OUTPUTS.find((it.first.as_wire()->name.index_)) != OUTPUTS.end()){
        //seq.insert(seq.begin(), (it.first.as_wire()->name.index_));
      }
      else{
        //seq.push_back((it.first.as_wire()->name.index_));
        //seq.push_back((it.second.as_wire()->name.index_));
      }

    }
    if (it.first.is_wire()) {
      out << "    nech" << (it.first.as_wire()->name.index_)<< "\n";
    }
    out << "    ch" << (it.first.is_chunk())<< " "<<(it.second.is_chunk())<< "\n";
    if (it.first.is_chunk() && it.second.is_chunk()){
      out << "    nech" << (it.first.as_chunk().wire)->name.index_<< " "<<(it.second.as_chunk().wire)->name.index_<< "\n";
      if (wires1.find(it.first.as_chunk().wire->name.index_)!=wires1.end()) {
        fout << wires1.find(it.first.as_chunk().wire->name.index_)->second<<"=";
      }
      if (INPUTS.find(it.first.as_chunk().wire->name.index_)!=INPUTS.end()) {
        fout << INPUTS.find(it.first.as_chunk().wire->name.index_)->second<<"=";
      }
      if (OUTPUTS.find(it.first.as_chunk().wire->name.index_)!=OUTPUTS.end()) {
        fout << OUTPUTS.find(it.first.as_chunk().wire->name.index_)->second<<"=";
      }
      if (wires1.find(it.second.as_chunk().wire->name.index_)!=wires1.end()) {
        fout << wires1.find(it.second.as_chunk().wire->name.index_)->second<<"\n";
      }
      if (INPUTS.find(it.second.as_chunk().wire->name.index_)!=INPUTS.end()) {
        fout << INPUTS.find(it.second.as_chunk().wire->name.index_)->second<<"\n";
      }
      if (OUTPUTS.find(it.second.as_chunk().wire->name.index_)!=OUTPUTS.end()) {
        fout << OUTPUTS.find(it.second.as_chunk().wire->name.index_)->second<<"\n";
      }
    }
    if (it.first.is_chunk()) {
      out << "    nech" << (it.first.as_chunk().wire->name.index_)<< "\n";
    }
    out << "    ch" << (it.first.is_fully_const()) <<" "<<(it.second.is_fully_const()) << "\n";
    if (it.first.is_fully_const() && it.second.is_fully_const()){
      out << "    nech" << (it.first.as_int())<< " "<<(it.second.as_int())<< "\n";
    }
    out << "    ch" << (it.first.is_fully_def()) <<" "<<(it.second.is_fully_def()) << "\n";
    //        if (it.first.is_fully_def() && it.second.is_fully_def()){
    //            out << "    nech" << (it.first.as_fully_def())<< " "<<(it.second.as_fully_def())<< "\n";
    //        }
    out << "    ch" << (it.first.is_fully_undef()) <<" "<<(it.second.is_fully_undef()) << "\n";
    //        if (it.first.is_fully_undef() && it.second.is_fully_undef()){
    //            out << "    nech" << (it.first.as_fully_undef())<< " "<<(it.second.as_fully_undef())<< "\n";
    //        }

  }
}
std::string st;
int temporary;
void print_syncs(const std::vector<Yosys::RTLIL::SyncRule *> &syncs, std::ostream &out, std::ofstream &fout){
  out << "  syncs\n";

  for (auto it1=syncs.begin(); it1 != syncs.end(); ++it1){
    std::vector<std::string> state;
    out << "    type " << sizeof((*it1)->type) << "\n";
    //enum Student_FirstName{ Denys, Rabbit, Milana, Olexandr };
    std::string Student_FirstNameString[]{"ST0", // level sensitive: 0
      "ST1", // level sensitive: 1
      "STp", // edge sensitive: posedge
      "STn", // edge sensitive: negedge
      "STe", // edge sensitive: both edges
      "STa", // always active
      "STi"  // init
    };
    std::cout << Student_FirstNameString[(*it1)->type] << std::endl;
    state.push_back(Student_FirstNameString[(*it1)->type]);

    if (Student_FirstNameString[(*it1)->type]=="STn"){
      st="negedge";
    }
    if (Student_FirstNameString[(*it1)->type]=="STp"){
      st="posedge";
    }
    if (Student_FirstNameString[(*it1)->type]=="ST0"){
      st="level0";
    }
    if (Student_FirstNameString[(*it1)->type]=="ST1"){
      st="level1";
    }


    out << "  signal\n";
    out << "    size " << (*it1)->signal.size() << "\n";
    out << "    as_wire index " << (*it1)->signal.as_wire()->name.index_ << "\n";
    temporary=(*it1)->signal.as_wire()->name.index_;
    out << "    as_chunk index " << (*it1)->signal.as_chunk().data.size() << "\n";;
    clk.push_back((*it1)->signal.as_wire()->name.index_);
    out << "  actions\n";
    if ((*it1)->actions.size()!=0){
      fout << "@(*) {\n";
    print_actions((*it1)->actions, out, fout);
    fout << "}\n";
    }
  }
 // out <<"zzzzzzzzzzss" <<INPUTS.size();


//  if(INPUTS.find(seq[2])!=INPUTS.end()){
//    fout << "\n@(" << st<<"("<<INPUTS.find(clk[0])->second << ")"<<") {\n    " << OUTPUTS.find(seq[0])->second << " = " << INPUTS.find(seq[2])->second << ";\n}\n";
//  }
//  else{
//    //fout << "\n@(" << st<<"("<<INPUTS.find(clk[0])->second << ")"<<") {\n    " << OUTPUTS.find(seq[0])->second << " = " << buildRIL(seq[2])<< ";\n}\n";
//  }
}

void printSigSpec(const std::vector<Yosys::RTLIL::SigSpec> &compare, std::ostream &out){
  out << "compare:\n";
  for (auto &it : compare){
    // z++;
    //   out << z <<"\n";
    out << "    ch" << (it.is_wire())<< " "<<(it.is_wire())<< "\n";
    if (it.is_wire() && it.is_wire()){
      out << "    nech" << (it.as_wire()->name.index_)<< " "<<(it.as_wire()->name.index_)<< "\n";
      if (OUTPUTS.find((it.as_wire()->name.index_)) != OUTPUTS.end()){
        seq.insert(seq.begin(), (it.as_wire()->name.index_));
      }
      else{
        seq.push_back((it.as_wire()->name.index_));
        seq.push_back((it.as_wire()->name.index_));
      }

    }
    out << "    ch" << (it.is_chunk())<< " "<<(it.is_chunk())<< "\n";
    if (it.is_chunk() && it.is_chunk()){
      out << "    nech" << (it.as_chunk().wire)->name.index_<< " "<<(it.as_chunk().wire)->name.index_<< "\n";
    }
    out << "    ch" << (it.is_fully_const()) <<" "<<(it.is_fully_const()) << "\n";
    //        if (it.first.is_fully_const() && it.second.is_fully_const()){
    //            out << "    nech" << (it.first.as_fully_const())<< " "<<(it.second.as_fully_const())<< "\n";
    //        }
    out << "    ch" << (it.is_fully_def()) <<" "<<(it.is_fully_def()) << "\n";
    //        if (it.first.is_fully_def() && it.second.is_fully_def()){
    //            out << "    nech" << (it.first.as_fully_def())<< " "<<(it.second.as_fully_def())<< "\n";
    //        }
    out << "    ch" << (it.is_fully_undef()) <<" "<<(it.is_fully_undef()) << "\n";
    //        if (it.first.is_fully_undef() && it.second.is_fully_undef()){
    //            out << "    nech" << (it.first.as_fully_undef())<< " "<<(it.second.as_fully_undef())<< "\n";
    //        }
  }
}
 void printCaseRule(std::vector< Yosys::RTLIL::CaseRule * > &cases, std::ostream &out, std::ofstream &fout);
void switches(std::vector< Yosys::RTLIL::SwitchRule* > 	&switches,  std::ostream &out, std::ofstream &fout){
  std::cout<<"\n\n    switches:\n\n";
  for (auto i: switches){
  std::cout << "sw:";
  auto it=i->signal;
  out << "    ch" << (it.is_wire())<< " "<<(it.is_wire())<< "\n";
  if (it.is_wire() && it.is_wire()){
    out << "    nech" << (it.as_wire()->name.index_)<< " "<<(it.as_wire()->name.index_)<< "\n";
    if (OUTPUTS.find((it.as_wire()->name.index_)) != OUTPUTS.end()){
      //seq.insert(seq.begin(), (it.as_wire()->name.index_));
    }
    else{
      //seq.push_back((it.as_wire()->name.index_));
      //seq.push_back((it.as_wire()->name.index_));
    }

  }
  out << "    ch" << (it.is_chunk())<< " "<<(it.is_chunk())<< "\n";
  if (it.is_chunk() && it.is_chunk()){
    out << "    nech" << (it.as_chunk().wire)->name.index_<< " "<<(it.as_chunk().wire)->name.index_<< "\n";
  }
  out << "    ch" << (it.is_fully_const()) <<" "<<(it.is_fully_const()) << "\n";
  //        if (it.first.is_fully_const() && it.second.is_fully_const()){
  //            out << "    nech" << (it.first.as_fully_const())<< " "<<(it.second.as_fully_const())<< "\n";
  //        }
  out << "    ch" << (it.is_fully_def()) <<" "<<(it.is_fully_def()) << "\n";
  //        if (it.first.is_fully_def() && it.second.is_fully_def()){
  //            out << "    nech" << (it.first.as_fully_def())<< " "<<(it.second.as_fully_def())<< "\n";
  //        }
  out << "    ch" << (it.is_fully_undef()) <<" "<<(it.is_fully_undef()) << "\n";
  //        if (it.first.is_fully_undef() && it.second.is_fully_undef()){
  //            out << "    nech" << (it.first.as_fully_undef())<< " "<<(it.second.as_fully_undef())<< "\n";
  //        }
  printCaseRule(i->cases,out,fout);
}
}
  void printCaseRule(std::vector< Yosys::RTLIL::CaseRule * > &cases, std::ostream &out, std::ofstream &fout){
    std::cout <<"CASERULE:\n";
    for (auto i: cases){
    printSigSpec(i->compare,out);
    print_actions(i->actions,out,fout);
    switches(i->switches,out,fout);
}
  }

void print_processes(const Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Process*> &processes, std::ostream &out, std::ofstream &fout){
  out << "Processes count: " << processes.size() << "\n";
  for (auto it1 = processes.begin(); it1 != processes.end(); ++it1){
    out << "  name " << it1->second->name.index_ << "\n";
    out << "  root_case: \n";
     print_syncs(it1->second->syncs, out, fout);
    fout<<"@("<<st<<"("<<INPUTS.find(temporary)->second<<")) {\n";
    printSigSpec((it1->second->root_case).compare, out);
    print_actions((it1->second->root_case).actions, out, fout);
    switches((it1->second->root_case).switches,out,fout );



//    for (auto &it2 : (it1->second->root_case).switches){
//      out << "    " << (it2->signal.as_chunk().wire)->name.index_ << "\n";
//      out << "    " << (it2->cases.size()) << "\n";

//      for (auto z: it2->signal.as_chunk().data){
//        std::string Student_FirstNameString[]{
//          "S0" ,
//          "S1" ,
//          "Sx", // undefined value or conflict
//          "Sz" , // high-impedance / not-connected
//          "Sa", // don't care (used only in cases)
//          "Sm"   // marker (used internally by some passes)
//        };
//        std::cout <<"type chunk" <<Student_FirstNameString[z] << std::endl;

//      }
//    }
    fout <<"}\n";

    out << "\n";
  }

}

void print_ports(const std::vector<Yosys::RTLIL::IdString> &ports, std::ostream &out, std::ofstream &fout){
  out << "Ports count: " << ports.size() << "\n";
  for (auto &it1 : ports){
    out << "  port " << it1.index_ << "\n";
  }
}

void print_params(const std::pair<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*> &m, std::ostream &out, std::ofstream &fout){
  eda::gate::model::GNet net(0);
  std::map<unsigned, Gate::Id> outputs;
  std::map<unsigned, Gate::Id> inputs;
  std::map<int, std::pair<int, int>> cell;
  std::map<int,GateSymbol> typeFunc;
  print_modules(m.first.index_, out, fout);
  out << "\n";
  print_wires(m.second->wires_, out, fout, net, inputs, outputs);
  out << "\n";
  print_cells(m.second->cells_, out, fout, net, typeFunc, cell);
  out << "\n" ;
  print_connections(m.second->connections_, out, fout, net, typeFunc, cell, inputs, outputs);
  out << "\n";
  print_memory(m.second->memories, out, fout);
  out << "\n";
  print_processes(m.second->processes, out, fout);
  out << "\n";
  print_ports(m.second->ports, out, fout);
  out << "\n";

  out << "Refcount wires count: " << m.second->refcount_wires_ << "\n";
  out << "Refcount cells count: " << m.second->refcount_cells_ << "\n";
  out << "Monitors count: " << m.second->monitors.size() << "\n";
  out << "Avail_parameters count: " << m.second->avail_parameters.size() << "\n";
  out << "\n";
}

void print_parsed(const Yosys::RTLIL::Design &des, std::ostream &out, std::ofstream &fout){
  for (auto &m: des.modules_){
    print_params(m, out, fout);
  }
}
int main(int argc, char* argv[]){
  std::ostream& out = std::cout;
  std::ofstream fout("temp.txt");
  Yosys::yosys_setup();
  for (size_t o=1;o<argc;++o){
    Yosys::RTLIL::Design design;
    Yosys::run_frontend(argv[o], "verilog", &design, nullptr);
    print_parsed(design, out, fout);
  }
  fout.close();
  Yosys::yosys_shutdown();
}
