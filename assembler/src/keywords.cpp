#include "../inc/parser.hpp"
#include <unordered_map>

std::unordered_map<std::string, TokenPtr> init() {
    std::unordered_map<std::string, TokenPtr> map;
    
    map.emplace("nop", std::make_unique<Opcode>(Opcode::Value::NOP));
    map.emplace("add", std::make_unique<Opcode>(Opcode::Value::ADD));
    map.emplace("adc", std::make_unique<Opcode>(Opcode::Value::ADC));
    map.emplace("sub", std::make_unique<Opcode>(Opcode::Value::SUB));
    map.emplace("sbc", std::make_unique<Opcode>(Opcode::Value::SBC));
    map.emplace("cmp", std::make_unique<Opcode>(Opcode::Value::CMP));
    map.emplace("cmc", std::make_unique<Opcode>(Opcode::Value::CMC));
    map.emplace("and", std::make_unique<Opcode>(Opcode::Value::AND));
    map.emplace("or", std::make_unique<Opcode>(Opcode::Value::OR));
    map.emplace("xor", std::make_unique<Opcode>(Opcode::Value::XOR));
    map.emplace("shl", std::make_unique<Opcode>(Opcode::Value::SHL));
    map.emplace("shr", std::make_unique<Opcode>(Opcode::Value::SHR));
    map.emplace("mov", std::make_unique<Opcode>(Opcode::Value::MOV));
    map.emplace("mvh", std::make_unique<Opcode>(Opcode::Value::MVH));
    map.emplace("tsb", std::make_unique<Opcode>(Opcode::Value::TSB));
    map.emplace("seb", std::make_unique<Opcode>(Opcode::Value::SEB));
    map.emplace("jmp", std::make_unique<Opcode>(Opcode::Value::JMP));
    map.emplace("rjmp", std::make_unique<Opcode>(Opcode::Value::RJMP));
    map.emplace("jbl", std::make_unique<Opcode>(Opcode::Value::JBL));
    map.emplace("jbh", std::make_unique<Opcode>(Opcode::Value::JBH));
    map.emplace("call", std::make_unique<Opcode>(Opcode::Value::CALL));
    map.emplace("rcall", std::make_unique<Opcode>(Opcode::Value::RCALL));
    map.emplace("cbl", std::make_unique<Opcode>(Opcode::Value::CBL));
    map.emplace("cbh", std::make_unique<Opcode>(Opcode::Value::CBH));
    map.emplace("ret", std::make_unique<Opcode>(Opcode::Value::RET));
    map.emplace("retcall", std::make_unique<Opcode>(Opcode::Value::RETCALL));
    map.emplace("ld", std::make_unique<Opcode>(Opcode::Value::LD));
    map.emplace("ldr", std::make_unique<Opcode>(Opcode::Value::LDR));
    map.emplace("lds", std::make_unique<Opcode>(Opcode::Value::LDS));
    map.emplace("ldf", std::make_unique<Opcode>(Opcode::Value::LDF));
    map.emplace("st", std::make_unique<Opcode>(Opcode::Value::ST));
    map.emplace("sts", std::make_unique<Opcode>(Opcode::Value::STS));
    map.emplace("stf", std::make_unique<Opcode>(Opcode::Value::STF));

    map.emplace("c", std::make_unique<Condition>(JumpCond::C, false));
    map.emplace("gteu", std::make_unique<Condition>(JumpCond::C, false));
    map.emplace("v", std::make_unique<Condition>(JumpCond::V, false));
    map.emplace("n", std::make_unique<Condition>(JumpCond::N, false));
    map.emplace("z", std::make_unique<Condition>(JumpCond::Z, false));
    map.emplace("eq", std::make_unique<Condition>(JumpCond::Z, false));
    map.emplace("gt", std::make_unique<Condition>(JumpCond::G, false));
    map.emplace("gte", std::make_unique<Condition>(JumpCond::GE, false));
    map.emplace("gtu", std::make_unique<Condition>(JumpCond::GU, false));
    map.emplace("nc", std::make_unique<Condition>(JumpCond::C, true));
    map.emplace("ltu", std::make_unique<Condition>(JumpCond::C, true));
    map.emplace("nv", std::make_unique<Condition>(JumpCond::V, true));
    map.emplace("nn", std::make_unique<Condition>(JumpCond::N, true));
    map.emplace("nz", std::make_unique<Condition>(JumpCond::Z, true));
    map.emplace("ne", std::make_unique<Condition>(JumpCond::Z, true));
    map.emplace("lte", std::make_unique<Condition>(JumpCond::G, true));
    map.emplace("lt", std::make_unique<Condition>(JumpCond::GE, true));
    map.emplace("lteu", std::make_unique<Condition>(JumpCond::GU, true));

    map.emplace("ra.l", std::make_unique<DataRegisterArg>(Register::RA_L));
    map.emplace("ra.h", std::make_unique<DataRegisterArg>(Register::RA_H));
    map.emplace("sr", std::make_unique<DataRegisterArg>(Register::SR));
    map.emplace("sp", std::make_unique<DataRegisterArg>(Register::SP));
    map.emplace("ga", std::make_unique<DataRegisterArg>(Register::GA));
    map.emplace("fp", std::make_unique<DataRegisterArg>(Register::GA));
    map.emplace("gb", std::make_unique<DataRegisterArg>(Register::GB));
    map.emplace("gc", std::make_unique<DataRegisterArg>(Register::GC));
    map.emplace("gd", std::make_unique<DataRegisterArg>(Register::GD));
    map.emplace("ge.l", std::make_unique<DataRegisterArg>(Register::GE_L));
    map.emplace("ge.h", std::make_unique<DataRegisterArg>(Register::GE_H));
    map.emplace("gf.l", std::make_unique<DataRegisterArg>(Register::GF_L));
    map.emplace("gf.h", std::make_unique<DataRegisterArg>(Register::GF_H));
    map.emplace("gg.l", std::make_unique<DataRegisterArg>(Register::GG_L));
    map.emplace("gg.h", std::make_unique<DataRegisterArg>(Register::GG_H));
    map.emplace("gh.l", std::make_unique<DataRegisterArg>(Register::GH_L));
    map.emplace("gh.h", std::make_unique<DataRegisterArg>(Register::GH_H));

    map.emplace("ge", std::make_unique<WideRegisterArg>(WideRegisterArg::Value::GE));
    map.emplace("gf", std::make_unique<WideRegisterArg>(WideRegisterArg::Value::GF));
    map.emplace("gg", std::make_unique<WideRegisterArg>(WideRegisterArg::Value::GG));
    map.emplace("gh", std::make_unique<WideRegisterArg>(WideRegisterArg::Value::GH));

    return map;
}

const std::unordered_map<std::string, TokenPtr> KEYWORDS(init());