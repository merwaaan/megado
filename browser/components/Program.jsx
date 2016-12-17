import React from 'react';
import classnames from 'classnames';
import _ from 'lodash';

function hex(x, size = 2) {
    return ('0'.repeat(size) + x.toString(16).toUpperCase()).substr(-size, size)
}

const memory = _.times(0x1000, _.random.bind(0x100)); // TODO use external data

const header =
    <tr>
        <td></td>
        <td>Opcode</td>
        <td>Instruction</td>
        <td>Operands</td>
    </tr>;

class Program extends React.Component {

    constructor(props) {
        super(props);

        this.state = {
            pc: 0
        };
    }

    render() {

        const rows = _.range(this.state.pc, this.state.pc + this.props.rowCount).map(offset => {

            let address = this.state.pc + offset;

            // Decode the instruction via the C API
            const instructionPtr = this.props.decode(this.props.genesis, address);
            //const name = instruction.split(' ')[0];
            //const operands = 'test';
            console.log(instructionPtr);

            console.log(Module.Pointer_stringify(instructionPtr));

            return <tr key={offset}>
                <td>{hex(offset, 8)}</td>
                <td>{_.join(_.slice(memory, offset, offset + _.random(1, 3)).map(h => hex(h, 2)), ' ')}</td>
                <td></td>
                <td>D1, (A4)</td>
            </tr>;
        });

        return (
            <div className="program">
                <table>
                    <tbody>
                        {header}
                        {rows}
                    </tbody>
                </table>
                <section className="controls">
                    <i className="fa fa-fw fa-play" onClick={null}></i>
                    <i className="fa fa-fw fa-pause" onClick={null}></i>
                    <i className="fa fa-fw fa-step-forward" onClick={this.handleStep.bind(this)}></i>
                </section>
            </div>);
    }

    handleStep() {
        console.log(`Stepping from @${this.state.pc}...`);
        this.state.pc = Module.ccall('genesis_step', 'number', ['number'], [this.props.genesis]);
        this.setState(this.state);
    }
}

Program.propTypes = {
    decode: React.PropTypes.func.isRequired,
    start: React.PropTypes.number.isRequired,
    rowCount: React.PropTypes.number.isRequired
};

export default Program;
