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

        };
    }

    render() {

        const rows = _.range(0, 20).map(offset => {

            // TODO decode via C API: const instruction = ...;

            return <tr key={offset}>
                <td>{hex(offset, 8)}</td>
                <td>{_.join(_.slice(memory, offset, offset + _.random(1, 3)).map(h => hex(h, 2)), ' ')}</td>
                <td>ADD.w</td>
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
                    <i className="fa fa-play" onClick={null}></i>
                    <i className="fa fa-pause" onClick={null}></i>
                    <i className="fa fa-step-forward" onClick={null}></i>
                </section>
            </div>);
    }
}

Program.propTypes = {
    rowCount: React.PropTypes.number.isRequired
};

export default Program;
