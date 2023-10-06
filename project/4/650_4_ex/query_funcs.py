from sqlalchemy import create_engine, and_, between

from sqlalchemy.orm import declarative_base
from sqlalchemy.orm import sessionmaker
from models import Player, Team, State, Color

Base = declarative_base()


def query1(
    C,
    use_mpg,
    min_mpg,
    max_mpg,
    use_ppg,
    min_ppg,
    max_ppg,
    use_rpg,
    min_rpg,
    max_rpg,
    use_apg,
    min_apg,
    max_apg,
    use_spg,
    min_spg,
    max_spg,
    use_bpg,
    min_bpg,
    max_bpg,
):
    Session = sessionmaker(bind=C)
    session = Session()

    filters = []

    if use_mpg:
        filters.append(between(Player.mpg, min_mpg, max_mpg))
    if use_ppg:
        filters.append(between(Player.ppg, min_ppg, max_ppg))
    if use_rpg:
        filters.append(between(Player.rpg, min_rpg, max_rpg))
    if use_apg:
        filters.append(between(Player.apg, min_apg, max_apg))
    if use_spg:
        filters.append(between(Player.spg, min_spg, max_spg))
    if use_bpg:
        filters.append(between(Player.bpg, min_bpg, max_bpg))

    results = session.query(Player).filter(and_(*filters)).all()

    print("PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG")
    for r in results:
        print(
            f"{r.player_id} {r.team_id} {r.uniform_num} {r.first_name} "
            f"{r.last_name} {r.mpg} {r.ppg} {r.rpg} {r.apg} "
            f"{r.spg:.1f} {r.bpg:.1f}"
        )


def query2(C, team_color):
    Session = sessionmaker(bind=C)
    session = Session()

    results = (
        session.query(Team.name)
        .join(Color, Team.color_id == Color.color_id)
        .filter(Color.name == team_color)
        .all()
    )

    print("NAME")
    for r in results:
        print(r[0])


def query3(C, team_name):
    Session = sessionmaker(bind=C)
    session = Session()

    results = (
        session.query(Player.first_name, Player.last_name)
        .join(Team, Player.team_id == Team.team_id)
        .filter(Team.name == team_name)
        .order_by(Player.ppg.desc())
        .all()
    )

    print("FIRST_NAME LAST_NAME")
    for r in results:
        print(f"{r[0]} {r[1]}")


def query4(C, team_state, team_color):
    Session = sessionmaker(bind=C)
    session = Session()

    results = (
        session.query(Player.uniform_num, Player.first_name, Player.last_name)
        .join(Team, Player.team_id == Team.team_id)
        .join(State, Team.state_id == State.state_id)
        .join(Color, Team.color_id == Color.color_id)
        .filter(and_(State.name == team_state, Color.name == team_color))
        .all()
    )

    print("UNIFORM_NUM FIRST_NAME LAST_NAME")
    for r in results:
        print(f"{r[0]} {r[1]} {r[2]}")


def query5(C, num_wins):
    Session = sessionmaker(bind=C)
    session = Session()

    results = (
        session.query(Player.first_name, Player.last_name, Team.name, Team.wins)
        .join(Team, Player.team_id == Team.team_id)
        .filter(Team.wins > num_wins)
        .all()
    )

    print("FIRST_NAME LAST_NAME NAME WINS")
    for r in results:
        print(f"{r[0]} {r[1]} {r[2]} {r[3]}")


def main():
    DATABASE_URL = "postgresql://postgres:3219@localhost/ACC_BBALL"
    C = create_engine(DATABASE_URL)

    # querytest
    # query1(C, 1, 15, 30, 1, 10, 25, 1, 5, 15, 1, 3, 10, 1, 1.0, 2.0, 1, 1.0, 2.0)
    # query1(C, 1, 35, 40, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2)
    query1(C, 1, 20, 30, 1, 10, 20, 1, 2, 5, 1, 1, 3, 1, 1, 2, 1, 0, 1)
    query1(C, 1, 35, 40, 0, 10, 20, 0, 2, 5, 0, 1, 3, 0, 1, 2, 0, 0, 1)
    query2(C, "Maroon")
    query3(C, "Duke")
    query4(C, "VA", "Maroon")
    query5(C, 10)


if __name__ == "__main__":
    main()
