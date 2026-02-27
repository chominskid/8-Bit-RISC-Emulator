#include "../inc/instruction.hpp"
#include "../../common/inc/encoding.hpp"
#include <cstddef>
#include <format>
#include <sstream>
#include <type_traits>
#include <unordered_map>

using ssize_t = std::make_signed_t<size_t>;

Encoder::Result::Result() :
    _size(0)
{}

Encoder::Result::Result(const std::string& str) :
    _size(ERROR),
    _error(std::make_unique<std::string>(str))
{}

Encoder::Result::Result(Result&& other) :
    _size(other._size)
{
    if (_size <= sizeof(_sdata))
        std::copy(other._sdata, &other._sdata[_size], _sdata);
    else if (_size != ERROR)
        std::construct_at(&_data, std::move(other._data));
    else
        std::construct_at(&_error, std::move(other._error));
}

Encoder::Result::~Result() {
    if (_size == ERROR)
        std::destroy_at(&_error);
    else if (_size > sizeof(_sdata))
        std::destroy_at(&_data);
}

bool Encoder::Result::has_value() const {
    return _size != ERROR;
}

size_t Encoder::Result::size() const {
    return _size;
}

std::string& Encoder::Result::error() {
    return *_error;
}
const std::string& Encoder::Result::error() const {
    return *_error;
}

uint8_t* Encoder::Result::begin() {
    if (_size <= sizeof(_sdata))
        return &_sdata[0];
    else
        return &_data[0];
}
const uint8_t* Encoder::Result::begin() const {
    if (_size <= sizeof(_sdata))
        return &_sdata[0];
    else
        return &_data[0];
}
const uint8_t* Encoder::Result::cbegin() const {
    if (_size <= sizeof(_sdata))
        return &_sdata[0];
    else
        return &_data[0];
}

uint8_t* Encoder::Result::end() {
    if (_size <= sizeof(_sdata))
        return &_sdata[_size];
    else
        return &_data[_size];
}
const uint8_t* Encoder::Result::end() const {
    if (_size <= sizeof(_sdata))
        return &_sdata[_size];
    else
        return &_data[_size];
}
const uint8_t* Encoder::Result::cend() const {
    if (_size <= sizeof(_sdata))
        return &_sdata[_size];
    else
        return &_data[_size];
}

Signature::Signature(Opcode::Value opcode, const std::vector<TokenPtr>& args) :
    arg_types(new Token::Type[args.size()]),
    num_args(args.size()),
    opcode(opcode)
{
    for (size_t i = 0; i < num_args; ++i)
        arg_types[i] = args[i]->type();
}

Signature::Signature(Opcode::Value opcode, const std::vector<Token::Type>& args) :
    arg_types(new Token::Type[args.size()]),
    num_args(args.size()),
    opcode(opcode)
{
    std::copy(args.begin(), args.end(), &arg_types[0]);
}

Signature::Signature(const Signature& other) :
    arg_types(new Token::Type[other.num_args]),
    num_args(other.num_args),
    opcode(other.opcode)
{
    std::copy(&other.arg_types[0], &other.arg_types[num_args], &arg_types[0]);
}

bool Signature::operator==(const Signature& other) const {
    if (opcode != other.opcode)
        return false;
    if (num_args != other.num_args)
        return false;
    for (size_t i = 0; i < num_args; ++i) {
        if (arg_types[i] != other.arg_types[i])
            return false;
    }
    return true;
}

void Encoder::Result::clear() {
    this->~Result();
    _size = 0;
}

void Encoder::Result::append(const Result& other) {
    if (_size == ERROR) {
        if (other._size == ERROR)
            _error->append(*other._error);
        return;
    } else if (other._size == ERROR) {
        this->~Result();
        std::construct_at(_error.get(), *other._error);
        _size = ERROR;
        return;
    }

    const size_t new_size = _size + other._size;
    if (new_size <= sizeof(_sdata))
        std::copy(other._sdata, &other._sdata[other._size], &_sdata[_size]);
    else {
        std::unique_ptr<uint8_t[]> new_data = std::make_unique<uint8_t[]>(new_size);
        std::copy(cbegin(), cend(), new_data.get());
        std::copy(other.cbegin(), other.cend(), &new_data[_size]);
        this->~Result();
        std::construct_at(&_data, std::move(new_data));
    }
    _size = new_size;
}

Encoder::Result& Encoder::Result::operator=(Result&& other) {
    this->~Result();
    _size = other._size;
    if (_size <= sizeof(_sdata))
        std::copy(other._sdata, &other._sdata[_size], _sdata);
    else if (_size != ERROR)
        std::construct_at(&_data, std::move(other._data));
    else
        std::construct_at(&_error, std::move(other._error));
    return *this;
}

std::string Signature::to_string() const {
    static const std::unordered_map<Token::Type, std::string> TYPE2STR {
        { Token::Type::OPCODE, "<opcode>" },
        { Token::Type::CONDITION, "<condition>" },
        { Token::Type::DATA_REGISTER, "<data register>" },
        { Token::Type::WIDE_REGISTER, "<wide register>" },
        { Token::Type::DIRECTIVE, "<directive>" },
        { Token::Type::INTEGER, "<integer>" },
        { Token::Type::LABEL, "<label>" },
        { Token::Type::LABEL_DECL, "<label declaration>" }
    };

    static const std::unordered_map<Opcode::Value, std::string> OPCODE2STR {
        { Opcode::Value::NOP, "nop" },
        { Opcode::Value::ADD, "add" },
        { Opcode::Value::ADC, "adc" },
        { Opcode::Value::SUB, "sub" },
        { Opcode::Value::SBC, "sbc" },
        { Opcode::Value::CMP, "cmp" },
        { Opcode::Value::CMC, "cmc" },
        { Opcode::Value::AND, "and" },
        { Opcode::Value::OR, "or" },
        { Opcode::Value::XOR, "xor" },
        { Opcode::Value::SHL, "shl" },
        { Opcode::Value::SHR, "shr" },
        { Opcode::Value::MOV, "mov" },
        { Opcode::Value::MVH, "mvh" },
        { Opcode::Value::TSB, "tsb" },
        { Opcode::Value::SEB, "seb" },
        { Opcode::Value::JMP, "jmp" },
        { Opcode::Value::RJMP, "rjmp" },
        { Opcode::Value::JBL, "jbl" },
        { Opcode::Value::JBH, "jbh" },
        { Opcode::Value::CALL, "call" },
        { Opcode::Value::RCALL, "rcall" },
        { Opcode::Value::CBL, "cbl" },
        { Opcode::Value::CBH, "cbh" },
        { Opcode::Value::RET, "ret" },
        { Opcode::Value::RETCALL, "retcall" },
        { Opcode::Value::LD, "ld" },
        { Opcode::Value::LDR, "ldr" },
        { Opcode::Value::LDS, "lds" },
        { Opcode::Value::LDF, "ldf" },
        { Opcode::Value::ST, "st" },
        { Opcode::Value::STS, "sts" },
        { Opcode::Value::STF, "stf" },
    };

    std::stringstream s;
    auto it = OPCODE2STR.find(opcode);
    s << (it == OPCODE2STR.end() ? "<?>" : it->second);
    for (size_t i = 0; i < num_args; ++i) {
        auto it = TYPE2STR.find(arg_types[i]);
        s << ' ' << (it == TYPE2STR.end() ? "<?>" : it->second);
    }
    return s.str();
}

size_t SignatureHasher::operator()(const Signature& signature) const {
    size_t hash = opcode_hasher(signature.opcode);
    for (const Token::Type* type = &signature.arg_types[0]; type != &signature.arg_types[signature.num_args]; ++type)
        hash ^= args_hasher(*type);
    return hash;
}

size_t SignatureHasher::operator()(const Instruction& instruction) const {
    return (*this)(instruction.signature);
}

Encoder::Result encode_reg_reg_alu(ALUOp op, const std::vector<TokenPtr>& args) {
    const uint16_t result =
        *op << *Encoding::O_SHIFT
        | *args[0]->get<DataRegisterArg>().value << *Encoding::X_SHIFT
        | *args[1]->get<DataRegisterArg>().value << *Encoding::Y_SHIFT;
    return result;
}

template <std::integral Int>
bool check_width_signed(Int value, int width) {
    using UInt = std::make_unsigned_t<Int>;
    const UInt x = value;

    const UInt MASK = ~(UInt)0 << (width - 1);
    const UInt REM = x & MASK;
    return REM == 0 || REM == MASK;
}

uint16_t encode_reg_imm_alu(ALUOp op, Register reg, uint8_t imm) {
    return *Encoding::FMT_IA << *Encoding::FMT_SHIFT
        | *op << *Encoding::O_SHIFT
        | ((imm << *Encoding::IH_SHIFT) & *Encoding::IH_MASK)
        | *reg << *Encoding::X_SHIFT
        | ((imm << *Encoding::IL_SHIFT) & *Encoding::IL_MASK);
}

Encoder::Result encode_reg_imm_alu(ALUOp op, const std::vector<TokenPtr>& args) {
    const std::optional<uint8_t> imm = args[1]->get<IntegerArg>().try_as<uint8_t>();
    if (!imm || !check_width_signed(*imm, *Encoding::I_WIDTH))
        return std::format("Immediate {} is too large for operand.", args[1]->get<IntegerArg>().to_string());
    return encode_reg_imm_alu(op, args[0]->get<DataRegisterArg>().value, *imm);
}

Encoder::Result encode_mov_reg_imm(Register reg, uint8_t imm) {
    if (check_width_signed(imm, *Encoding::I_WIDTH))
        return encode_reg_imm_alu(ALUOp::MOV, reg, imm);

    const uint16_t first = *Encoding::FMT_IA << *Encoding::FMT_SHIFT
        | *ALUOp::MOV << *Encoding::O_SHIFT
        | ((imm << *Encoding::IH_SHIFT) & *Encoding::IH_MASK)
        | *reg << *Encoding::X_SHIFT
        | ((imm << *Encoding::IL_SHIFT) & *Encoding::IL_MASK);
    
    imm >>= *Encoding::I_WIDTH;
    
    const uint16_t second = *Encoding::FMT_IA << *Encoding::FMT_SHIFT
        | *ALUOp::MOVH << *Encoding::O_SHIFT
        | ((imm << *Encoding::IH_SHIFT) & *Encoding::IH_MASK)
        | *reg << *Encoding::X_SHIFT
        | ((imm << *Encoding::IL_SHIFT) & *Encoding::IL_MASK);

    Encoder::Result result;
    result.append(first);
    result.append(second);
    return result;
}

Encoder::Result encode_mov_reg_imm(const std::vector<TokenPtr>& args) {
    const Register reg = args[0]->get<DataRegisterArg>().value;
    const std::optional<uint8_t> imm = args[1]->get<IntegerArg>().try_as<uint8_t>();
    if (!imm)
        return std::format("Immediate {} is too large for immediate load to data register.", args[1]->get<IntegerArg>().to_string());
    return encode_mov_reg_imm(reg, *imm);
}

Encoder::Result encode_mov_wreg_imm(WideRegisterArg::Value reg, uint16_t imm) {
    const Register reg_low = (Register)(*Register::GE_L + *reg * 2);
    const Register reg_high = (Register)(*reg_low + 1);

    Encoder::Result result;
    result.append(encode_mov_reg_imm(reg_low, imm));
    result.append(encode_mov_reg_imm(reg_high, imm >> 8));
    return result;
}

Encoder::Result encode_mov_wreg_label(const std::vector<TokenPtr>& args) {
    const WideRegisterArg::Value reg = args[0]->get<WideRegisterArg>().value;
    const size_t address = args[1]->get<LabelArg>().address;
    if (!check_width_signed(address, 16))
        return std::format("Address {} (for label \"{}\") is too large for immediate load to wide register.", address, args[1]->get<LabelArg>().value);
    return encode_mov_wreg_imm(reg, address);
}

Encoder::Result encode_mov_wreg_imm(const std::vector<TokenPtr>& args) {
    const std::optional<uint16_t> imm = args[1]->get<IntegerArg>().try_as<uint16_t>();
    if (!imm)
        return std::format("Immediate {} is too large for immediate load to wide register.", args[1]->get<IntegerArg>().to_string());
    return encode_mov_wreg_imm(args[0]->get<WideRegisterArg>().value, *imm);
}

Encoder::Result encode_reg_imm_shift(ALUOp op, const std::vector<TokenPtr>& args) {
    const std::optional<uint8_t> imm = args[1]->get<IntegerArg>().try_as<uint8_t>();
    if (!imm)
        return std::format("Immediate {} is too large for bitwise shift.", args[1]->get<IntegerArg>().to_string());

    if (*imm > 7)
        return std::format("Immediate {} is too large for bitwise shift.", (int)(int8_t)*imm);

    const uint16_t result =
        *Encoding::FMT_IA << *Encoding::FMT_SHIFT
        | *op << *Encoding::O_SHIFT
        | ((*imm << *Encoding::IH_SHIFT) & *Encoding::IH_MASK)
        | *args[0]->get<DataRegisterArg>().value << *Encoding::X_SHIFT
        | ((*imm << *Encoding::IL_SHIFT) & *Encoding::IL_MASK);
    
    return result;
}

AddrModeC get_addr_mode_c(WideRegisterArg::Value reg) {
    return (AddrModeC)(*reg - *WideRegisterArg::Value::GE + *AddrModeC::GE);
}

AddrModeM get_addr_mode_m(WideRegisterArg::Value reg) {
    return (AddrModeM)(*reg - *WideRegisterArg::Value::GE + *AddrModeM::GE);
}

uint16_t encode_jmp(bool call, AddrModeC mode, bool negate, JumpCond cond, uint8_t offset) {
    return *Encoding::FMT_C << *Encoding::FMT_SHIFT
        | call << *Encoding::S_SHIFT
        | *mode << *Encoding::M_SHIFT
        | ((offset << *Encoding::IH_SHIFT) & *Encoding::IH_MASK)
        | negate << *Encoding::N_SHIFT
        | *cond << *Encoding::C_SHIFT
        | ((offset << *Encoding::IL_SHIFT) & *Encoding::IL_MASK);
}

Encoder::Result encode_jmp_wreg(bool call, const TokenPtr& reg_arg) {
    const AddrModeC mode = get_addr_mode_c(reg_arg->get<WideRegisterArg>().value);

    const uint16_t result =
        *Encoding::FMT_C << *Encoding::FMT_SHIFT
        | call << *Encoding::S_SHIFT
        | *mode << *Encoding::M_SHIFT
        | *JumpCond::ALW << *Encoding::C_SHIFT;

    return result;
}

Encoder::Result encode_jmp_wreg_off(bool call, const TokenPtr& reg_arg, const TokenPtr& off_arg) {
    const AddrModeC mode = get_addr_mode_c(reg_arg->get<WideRegisterArg>().value);

    const std::optional<size_t> offset = off_arg->get<IntegerArg>().try_as<size_t>();
    if (!offset)
        return std::format("Immediate {} is too large for jump offset.", off_arg->get<IntegerArg>().to_string());

    const size_t MASK = ~(size_t)0 << (*Encoding::I_WIDTH - 1);
    const size_t REM = *offset & MASK;
    if (REM != 0 && REM != MASK)
        return std::format("Immediate {} is too large for jump offset.", (ssize_t)*offset);

    const uint16_t result =
        *Encoding::FMT_C << *Encoding::FMT_SHIFT
        | call << *Encoding::S_SHIFT
        | *mode << *Encoding::M_SHIFT
        | ((*offset << *Encoding::IH_SHIFT) & *Encoding::IH_MASK)
        | *JumpCond::ALW << *Encoding::C_SHIFT
        | ((*offset << *Encoding::IL_SHIFT) & *Encoding::IL_MASK);

    return result;
}

Encoder::Result encode_rjmp_label(bool call, size_t address, const TokenPtr& label_arg) {
    size_t offset = label_arg->get<LabelArg>().address - address - 2;
    if (address % 2 != 0)
        return std::format("Offset {} is not aligned to a 2-byte boundary.", (ssize_t)offset);
    offset = (ssize_t)offset / 2;

    const size_t MASK = ~(size_t)0 << (*Encoding::I_WIDTH - 1);
    const size_t REM = offset & MASK;
    if (REM != 0 && REM != MASK)
        return std::format("Offset {} is too large for a relative jump.", (ssize_t)offset);

    const uint16_t result =
        *Encoding::FMT_C << *Encoding::FMT_SHIFT
        | call << *Encoding::S_SHIFT
        | *AddrModeC::REL << *Encoding::M_SHIFT
        | ((offset << *Encoding::IH_SHIFT) & *Encoding::IH_MASK)
        | *JumpCond::ALW << *Encoding::C_SHIFT
        | ((offset << *Encoding::IL_SHIFT) & *Encoding::IL_MASK);

    return result;
}

Encoder::Result encode_jmp_wreg(bool call, bool negate, JumpCond cond, const TokenPtr& reg_arg) {
    const AddrModeC mode = get_addr_mode_c(reg_arg->get<WideRegisterArg>().value);

    const uint16_t result =
        *Encoding::FMT_C << *Encoding::FMT_SHIFT
        | call << *Encoding::S_SHIFT
        | *mode << *Encoding::M_SHIFT
        | negate << *Encoding::N_SHIFT
        | *cond << *Encoding::C_SHIFT;

    return result;
}

Encoder::Result encode_jmp_wreg_off(bool call, bool negate, JumpCond cond, const TokenPtr& reg_arg, const TokenPtr& off_arg) {
    const AddrModeC mode = get_addr_mode_c(reg_arg->get<WideRegisterArg>().value);

    const std::optional<size_t> offset = off_arg->get<IntegerArg>().try_as<size_t>();
    if (!offset)
        return std::format("Immediate {} is too large for jump offset.", off_arg->get<IntegerArg>().to_string());

    const size_t MASK = ~(size_t)0 << (*Encoding::I_WIDTH - 1);
    const size_t REM = *offset & MASK;
    if (REM != 0 && REM != MASK)
        return std::format("Immediate {} is too large for jump offset.", (ssize_t)*offset);

    const uint16_t result =
        *Encoding::FMT_C << *Encoding::FMT_SHIFT
        | call << *Encoding::S_SHIFT
        | *mode << *Encoding::M_SHIFT
        | ((*offset << *Encoding::IH_SHIFT) & *Encoding::IH_MASK)
        | negate << *Encoding::N_SHIFT
        | *cond << *Encoding::C_SHIFT
        | ((*offset << *Encoding::IL_SHIFT) & *Encoding::IL_MASK);

    return result;
}

Encoder::Result encode_rjmp_label(bool call, size_t address, bool negate, JumpCond cond, const TokenPtr& label_arg) {
    size_t offset = label_arg->get<LabelArg>().address - address - 2;
    if (address % 2 != 0)
        return std::format("Offset {} is not aligned to a 2-byte boundary.", (ssize_t)offset);
    offset = (ssize_t)offset / 2;

    const size_t MASK = ~(size_t)0 << (*Encoding::I_WIDTH - 1);
    const size_t REM = offset & MASK;
    if (REM != 0 && REM != MASK)
        return std::format("Offset {} is too large for a relative jump.", (ssize_t)offset);

    const uint16_t result =
        *Encoding::FMT_C << *Encoding::FMT_SHIFT
        | call << *Encoding::S_SHIFT
        | *AddrModeC::REL << *Encoding::M_SHIFT
        | ((offset << *Encoding::IH_SHIFT) & *Encoding::IH_MASK)
        | negate << *Encoding::N_SHIFT
        | *cond << *Encoding::C_SHIFT
        | ((offset << *Encoding::IL_SHIFT) & *Encoding::IL_MASK);

    return result;
}

uint16_t encode_mem(bool store, Register reg, AddrModeM mode, uint8_t offset) {
    return *Encoding::FMT_M << *Encoding::FMT_SHIFT
        | store << *Encoding::S_SHIFT
        | *mode << *Encoding::M_SHIFT
        | ((offset << *Encoding::IH_SHIFT) & *Encoding::IH_MASK)
        | *reg << *Encoding::X_SHIFT
        | ((offset << *Encoding::IL_SHIFT) & *Encoding::IL_MASK);
}

Encoder::Result encode_mem(bool store, AddrModeM mode, const std::vector<TokenPtr>& args) {
    return encode_mem(store, args[0]->get<DataRegisterArg>().value, mode, 0);
}

Encoder::Result encode_mem_off(bool store, AddrModeM mode, const std::vector<TokenPtr>& args) {
    const std::optional<uint8_t> offset = args[2]->get<IntegerArg>().try_as<uint8_t>();
    if (!offset || !check_width_signed(*offset, *Encoding::I_WIDTH))
        return std::format("Immediate {} is too large for memory offset.", args[2]->get<IntegerArg>().to_string());

    return encode_mem(store, args[0]->get<DataRegisterArg>().value, mode, *offset);
}

Encoder::Result encode_mem_wreg_off(bool store, const std::vector<TokenPtr>& args) {
    return encode_mem_off(store, get_addr_mode_m(args[1]->get<WideRegisterArg>().value), args);
}

Encoder::Result encode_mem_wreg(bool store, const std::vector<TokenPtr>& args) {
    return encode_mem(store, get_addr_mode_m(args[1]->get<WideRegisterArg>().value), args);
}

const std::unordered_set<Instruction, SignatureHasher, SignatureEqual> INSTRUCTIONS {
    Instruction {
        .signature = { Opcode::Value::NOP },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>&) -> Encoder::Result {
                return (uint16_t)(
                    *ALUOp::MOV << *Encoding::O_SHIFT
                    | *Register::GB << *Encoding::X_SHIFT
                    | *Register::GB << *Encoding::Y_SHIFT
                );
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::ADD, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::ADD, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::ADC, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::ADC, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::SUB, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::SUB, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::SBC, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::SBC, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::CMP, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::CMP, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::CMC, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::CMC, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::AND, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::AND, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::OR, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::OR, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::XOR, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::XOR, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::SHL, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::SHL, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::SHR, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::SHR, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::MOV, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::MOV, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::MVH, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::MOVH, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::TSB, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::TSB, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::SEB, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::SEB, args);
            }},
        }
    },

    Instruction {
        .signature = { Opcode::Value::ADD, { Token::Type::DATA_REGISTER, Token::Type::INTEGER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_imm_alu(ALUOp::ADD, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::ADC, { Token::Type::DATA_REGISTER, Token::Type::INTEGER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_imm_alu(ALUOp::ADC, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::SUB, { Token::Type::DATA_REGISTER, Token::Type::INTEGER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_imm_alu(ALUOp::SUB, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::SBC, { Token::Type::DATA_REGISTER, Token::Type::INTEGER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_imm_alu(ALUOp::SBC, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::CMP, { Token::Type::DATA_REGISTER, Token::Type::INTEGER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_imm_alu(ALUOp::CMP, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::CMC, { Token::Type::DATA_REGISTER, Token::Type::INTEGER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_imm_alu(ALUOp::CMC, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::AND, { Token::Type::DATA_REGISTER, Token::Type::INTEGER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_imm_alu(ALUOp::AND, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::OR, { Token::Type::DATA_REGISTER, Token::Type::INTEGER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_imm_alu(ALUOp::OR, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::XOR, { Token::Type::DATA_REGISTER, Token::Type::INTEGER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_imm_alu(ALUOp::XOR, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::SHL, { Token::Type::DATA_REGISTER, Token::Type::INTEGER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_imm_shift(ALUOp::SHL, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::SHR, { Token::Type::DATA_REGISTER, Token::Type::INTEGER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_imm_shift(ALUOp::SHR, args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::MOV, { Token::Type::DATA_REGISTER, Token::Type::INTEGER } },
        .independent = true,
        .encoders = {
            { .size = std::nullopt, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mov_reg_imm(args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::MOV, { Token::Type::WIDE_REGISTER, Token::Type::INTEGER } },
        .independent = true,
        .encoders = {
            { .size = std::nullopt, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mov_wreg_imm(args);
            }},
        }
    },
    Instruction {
        .signature = { Opcode::Value::MOV, { Token::Type::WIDE_REGISTER, Token::Type::LABEL }},
        .independent = false,
        .encoders = {
            { .size = std::nullopt, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mov_wreg_label(args);
            }}
        }
    },
    Instruction {
        .signature = { Opcode::Value::MVH, { Token::Type::DATA_REGISTER, Token::Type::INTEGER } },
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_reg_reg_alu(ALUOp::MOVH, args);
            }},
        }
    },
    // Instruction {
    //     .signature = { Opcode::Value::TSB, { Token::Type::DATA_REGISTER, Token::Type::INTEGER, Token::Type::INTEGER } },
    //     .independent = true,
    //     .encoders = {
    //         { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                
    //         }},
    //     }
    // },
    // Instruction {
    //     .signature = { Opcode::Value::SEB, { Token::Type::DATA_REGISTER, Token::Type::DATA_REGISTER } },
    //     .independent = true,
    //     .encoders = {
    //         { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
    //             return encode_reg_reg_alu(ALUOp::SEB, args);
    //         }},
    //     }
    // },

    Instruction {
        .signature = { Opcode::Value::JMP, { Token::Type::WIDE_REGISTER }},
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_jmp_wreg(false, args[0]);
            }
        }}
    },
    Instruction {
        .signature = { Opcode::Value::JMP, { Token::Type::WIDE_REGISTER, Token::Type::INTEGER }},
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_jmp_wreg_off(false, args[0], args[1]);
            }
        }}
    },
    Instruction {
        .signature = { Opcode::Value::CALL, { Token::Type::WIDE_REGISTER }},
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_jmp_wreg(true, args[0]);
            }
        }}
    },
    Instruction {
        .signature = { Opcode::Value::CALL, { Token::Type::WIDE_REGISTER, Token::Type::INTEGER }},
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_jmp_wreg_off(true, args[0], args[1]);
            }
        }}
    },
    Instruction {
        .signature = Signature(Opcode::Value::RJMP, { Token::Type::LABEL }),
        .independent = false,
        .encoders = {
            { .size = 2, .encode = [] (size_t address, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_rjmp_label(false, address, args[0]);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::RCALL, { Token::Type::LABEL }),
        .independent = false,
        .encoders = {
            { .size = 2, .encode = [] (size_t address, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_rjmp_label(true, address, args[0]);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::RET),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>&) -> Encoder::Result {
                return encode_jmp(false, AddrModeC::RET, false, JumpCond::ALW, 0);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::RETCALL),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>&) -> Encoder::Result {
                return encode_jmp(true, AddrModeC::RET, false, JumpCond::ALW, 0);
            }},
        }
    },

    Instruction {
        .signature = { Opcode::Value::JMP, { Token::Type::CONDITION, Token::Type::WIDE_REGISTER }},
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                const auto& cond = args[0]->get<Condition>();
                return encode_jmp_wreg(false, cond.negate, cond.cond, args[1]);
            }
        }}
    },
    Instruction {
        .signature = { Opcode::Value::JMP, { Token::Type::CONDITION, Token::Type::WIDE_REGISTER, Token::Type::INTEGER }},
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                const auto& cond = args[0]->get<Condition>();
                return encode_jmp_wreg_off(false, cond.negate, cond.cond, args[0], args[1]);
            }
        }}
    },
    Instruction {
        .signature = { Opcode::Value::CALL, { Token::Type::CONDITION, Token::Type::WIDE_REGISTER }},
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                const auto& cond = args[0]->get<Condition>();
                return encode_jmp_wreg(true, cond.negate, cond.cond, args[0]);
            }
        }}
    },
    Instruction {
        .signature = { Opcode::Value::CALL, { Token::Type::CONDITION, Token::Type::WIDE_REGISTER, Token::Type::INTEGER }},
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                const auto& cond = args[0]->get<Condition>();
                return encode_jmp_wreg_off(true, cond.negate, cond.cond, args[1], args[2]);
            }
        }}
    },
    Instruction {
        .signature = Signature(Opcode::Value::RJMP, { Token::Type::CONDITION, Token::Type::LABEL }),
        .independent = false,
        .encoders = {
            { .size = 2, .encode = [] (size_t address, const std::vector<TokenPtr>& args) -> Encoder::Result {
                const auto& cond = args[0]->get<Condition>();
                return encode_rjmp_label(false, address, cond.negate, cond.cond, args[1]);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::RCALL, { Token::Type::CONDITION, Token::Type::LABEL }),
        .independent = false,
        .encoders = {
            { .size = 2, .encode = [] (size_t address, const std::vector<TokenPtr>& args) -> Encoder::Result {
                const auto& cond = args[0]->get<Condition>();
                return encode_rjmp_label(true, address, cond.negate, cond.cond, args[1]);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::RET, { Token::Type::CONDITION }),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                const auto& cond = args[0]->get<Condition>();
                return encode_jmp(false, AddrModeC::RET, cond.negate, cond.cond, 0);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::RETCALL, { Token::Type::CONDITION }),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                const auto& cond = args[0]->get<Condition>();
                return encode_jmp(true, AddrModeC::RET, cond.negate, cond.cond, 0);
            }},
        }
    },

    Instruction {
        .signature = Signature(Opcode::Value::LD, { Token::Type::DATA_REGISTER, Token::Type::WIDE_REGISTER }),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mem_wreg(false, args);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::LD, { Token::Type::DATA_REGISTER, Token::Type::WIDE_REGISTER, Token::Type::INTEGER }),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mem_wreg_off(false, args);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::LDS, { Token::Type::DATA_REGISTER, Token::Type::WIDE_REGISTER }),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mem(false, AddrModeM::STACK, args);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::LDS, { Token::Type::DATA_REGISTER, Token::Type::WIDE_REGISTER, Token::Type::INTEGER }),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mem_off(false, AddrModeM::STACK, args);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::LDF, { Token::Type::DATA_REGISTER, Token::Type::WIDE_REGISTER }),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mem(false, AddrModeM::FRAME, args);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::LDF, { Token::Type::DATA_REGISTER, Token::Type::WIDE_REGISTER, Token::Type::INTEGER }),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mem_off(false, AddrModeM::FRAME, args);
            }},
        }
    },

    Instruction {
        .signature = Signature(Opcode::Value::ST, { Token::Type::DATA_REGISTER, Token::Type::WIDE_REGISTER }),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mem_wreg(true, args);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::ST, { Token::Type::DATA_REGISTER, Token::Type::WIDE_REGISTER, Token::Type::INTEGER }),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mem_wreg_off(true, args);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::STS, { Token::Type::DATA_REGISTER, Token::Type::WIDE_REGISTER }),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mem(true, AddrModeM::STACK, args);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::STS, { Token::Type::DATA_REGISTER, Token::Type::WIDE_REGISTER, Token::Type::INTEGER }),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mem_off(true, AddrModeM::STACK, args);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::STF, { Token::Type::DATA_REGISTER, Token::Type::WIDE_REGISTER }),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mem(true, AddrModeM::FRAME, args);
            }},
        }
    },
    Instruction {
        .signature = Signature(Opcode::Value::STF, { Token::Type::DATA_REGISTER, Token::Type::WIDE_REGISTER, Token::Type::INTEGER }),
        .independent = true,
        .encoders = {
            { .size = 2, .encode = [] (size_t, const std::vector<TokenPtr>& args) -> Encoder::Result {
                return encode_mem_off(true, AddrModeM::FRAME, args);
            }},
        }
    },
};