import React from 'react';
import classnames from 'classnames';
import _ from 'lodash';

function hex(x, size = 2) {
    return ('0'.repeat(size) + x.toString(16).toUpperCase()).substr(-size, size)
}

const memory = _.times(0x1000, _.random.bind(0x100)); // TODO use external data

const decodeInstruction = Module.cwrap('m68k_decode', 'number', ['number', 'number']);

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

        };
    }

    render() {

        const rows = _.range(0, this.props.rowCount).map(offset => {

            let address = this.props.pc + offset;

            // Decode the instruction via the C API
            const instructionPtr = decodeInstruction();
            //const name = instruction.split(' ')[0];
            //const operands = 'test';

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
                    <i className="fa fa-fw fa-step-forward" onClick={null}></i>
                </section>
            </div>);
    }
}

Program.propTypes = {
    start: React.PropTypes.number.isRequired,
    rowCount: React.PropTypes.number.isRequired
};

export default Program;
