#include <iostream>
#include <fstream>
#include <vector>

#include <AIToolbox/Utils/Core.hpp>
#include <AIToolbox/Factored/Utils/Core.hpp>
#include <AIToolbox/Tools/Statistics.hpp>

#include <CommandLineParsing.hpp>
#include <NodesProblem.hpp>

namespace f = AIToolbox::Factored;

int main(int argc, char** argv) {
    size_t nodes;
    unsigned experiments;
    unsigned timesteps;
    std::string filename;
    Options options;
    options.push_back(makeRequiredOption("nodes,n", &nodes, "set the experiment's nodes number"));
    options.push_back(makeDefaultedOption("experiments,e", &experiments, "set the number of experiments", 1000u));
    options.push_back(makeDefaultedOption("timesteps,t", &timesteps, "set the timesteps per experiment", 500u));
    options.push_back(makeRequiredOption("output,o", &filename, "set the final output file"));

    if (!parseCommandLine(argc, argv, options))
        return 1;
    if (nodes < 3) {
        std::cout << "Nodes cannot be less than three!\n";
        return 1;
    }

    f::Action A(nodes, 2);
    f::Rewards rew(nodes - 1);
    std::default_random_engine actionRand(nodes);
    std::bernoulli_distribution actionDist(0.5);

    // const double factorsNum = A.size() - 1.0;

    auto [getReward, getRegret, norm] = makeNodesProblem(nodes);

    AIToolbox::Statistics regrets(timesteps);

    for (unsigned e = 0; e < experiments; ++e) {
        std::cout << "Experiment " << e+1 << std::endl;
        f::Action action(A.size(), 0);

        for (unsigned t = 0; t < timesteps; ++t) {
            // printAction(action);
            double trueRegret = 0.0;
            for (unsigned i = 0; i < nodes-1; ++i) {
                rew[i]   = getReward(i, action[i], action[i+1]);
                trueRegret += getRegret(i, action[i], action[i+1]);
            }
            // std::cout << " ==> " << rew[0];
            // for (size_t q = 1; q < nodes-1; ++q) std::cout << ", " << rew[q];
            // std::cout << "\n";
            // const double expRegret = 1.0 - rew.sum()/norm;
            regrets.record(trueRegret / norm, t);

            for (size_t i = 0; i < action.size(); ++i)
                action[i] = actionDist(actionRand);
        }
    }

    std::ofstream file(filename);
    file << regrets;
}
