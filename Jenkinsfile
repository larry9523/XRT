#!/usr/bin/env groovy

boolean prBuild = env.ghprbPullLink != null;
env.WORKSPACE = params.DEV ? "/proj/rdi/buildsD/xbb/XRT_IPU_DEV/" : "/proj/rdi/buildsD/xbb/XRT_IPU/"
env.AUTH_TOKEN = params.AUTH_TOKEN
env.IS_CI = params.DEV ? false : true
/**
 * sync the git WS.
 * @param prBuild if true then it will get the PR to the sandbox, otherwise the commit
 * 
 */

def syncWS(boolean prBuild) {
    def scmVars
    if (prBuild) {
        scmVars = checkout([$class: 'GitSCM',
            branches: [
                [name: "${env.sha1}"]
            ],
            doGenerateSubmoduleConfigurations: false,
            extensions: [
                [$class: 'WipeWorkspace'],
                [$class: 'CheckoutOption', timeout: 30],
                [$class: 'RelativeTargetDirectory', 'relativeTargetDir': "${env.WORKSPACE}"],
            ],
            submoduleCfg: [],
            userRemoteConfigs: [
                [credentialsId: 'xbuild-ml-frontend-gitenterprise-token',
                    name: 'origin',
                    refspec: "+refs/heads/*:refs/remotes/origin/* +refs/pull/*:refs/remotes/origin/pr/*",
                    url: "${env.GITPROJECTURL}"
                ]
            ]
        ])

    } else {
        scmVars = checkout([$class: 'GitSCM',
            branches: [
                [name: "*/" + "${env.GITBRANCH}"]
            ],
            doGenerateSubmoduleConfigurations: false,
            extensions: [
                [$class: 'WipeWorkspace'],
                [$class: 'CheckoutOption', timeout: 30],
                [$class: 'RelativeTargetDirectory', 'relativeTargetDir': "${env.WORKSPACE}"],
            ],
            submoduleCfg: [],
            userRemoteConfigs: [
                [credentialsId: 'xbuild-ml-frontend-gitenterprise-token',
                    name: 'origin',
                    url: "${env.GITPROJECTURL}"
                ]
            ]
        ])

    }

}

def runBuild(docker_container_name) {
    return 'docker-compose run --rm ' + "${docker_container_name}" + ' "cd build/gradle; ./gradlew buildXRT --project-cache-dir=/tmp/' + "${docker_container_name}" + '"'
}

def publishDeb(docker_container_name) {
    return 'docker-compose run --rm ' + "${docker_container_name}" + ' "cd build/gradle; ./gradlew publishDeb --project-cache-dir=/tmp/' + "${docker_container_name}" + '"'
}

def publishRpm(docker_container_name) {
    return 'docker-compose run --rm ' + "${docker_container_name}" + ' "cd build/gradle; ./gradlew publishRpm --project-cache-dir=/tmp/' + "${docker_container_name}" + '"'
}

pipeline {

    agent {
        label 'AIEIPUPOOL'
    }

    stages {
        stage('Sync Workspace') {
            steps {
                syncWS(prBuild)
            }
        }

        stage("Build") {
            parallel {
                stage('Build Ubuntu20.04') {
                    steps {
                        timeout(time: 1, unit: 'HOURS') {
                            dir("${env.WORKSPACE}" + "/build/docker") {

                                catchError {
                                    script {
                                          sh runBuild("xrt-ipu-ubuntu2004")
                                    }
                                }
                            }
                        }
                    }
                }

                stage('Build Centos 7.6') {
                    steps {
                        timeout(time: 1, unit: 'HOURS') {
                            dir("${env.WORKSPACE}" + "/build/docker") {

                                catchError {
                                    script {
                                        sh runBuild("xrt-ipu-centos76")
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        stage('Test') {
            steps {
                timeout(time: 1, unit: 'HOURS') {
                    echo "Running Tests"
                }
            }
        }

        stage('Publish') {
            parallel {
                stage('Publish Ubuntu20.04') {
                    steps {
                        timeout(time: 1, unit: 'HOURS') {
                            script {
                                if (prBuild || params.DEV) {
                                    echo "Skipping the Adding artifacts to Artifactory Step"
                                } else {
                                    dir("${env.WORKSPACE}" + "/build/docker") {
                                        catchError {
                                            withCredentials([usernamePassword(credentialsId: env.AUTH_TOKEN, usernameVariable: 'USER', passwordVariable: 'AUTH_TOKEN')]) {
                                                withEnv(["AUTH_TOKEN=${AUTH_TOKEN}", "USER=${USER}"]) {
                                            sh publishDeb("xrt-ipu-ubuntu2004")
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                stage('Publish Centos7.6') {
                    steps {
                        timeout(time: 1, unit: 'HOURS') {
                            script {
                                if (prBuild || params.DEV) {
                                    echo "Skipping the Adding artifacts to Artifactory Step"
                                } else {
                                    dir("${env.WORKSPACE}" + "/build/docker") {
                                        catchError {
                                         withCredentials([usernamePassword(credentialsId: env.AUTH_TOKEN, usernameVariable: 'USER', passwordVariable: 'AUTH_TOKEN')]) {
                                             withEnv(["AUTH_TOKEN=${AUTH_TOKEN}", "USER=${USER}"]) {
                                            sh publishRpm("xrt-ipu-centos76")
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    }
    post {
        // Clean after build
        always {
            dir("${env.WORKSPACE}") {
                deleteDir()
            }
            cleanWs()
        }
    }
}
